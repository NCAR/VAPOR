import sys, os
from glob import glob
from shutil import copyfile
import fnmatch
import yaml, pprint
import subprocess
import jinja2
from itertools import *
import concurrent.futures

def ParseAruments() -> (dict, dict):
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('configPath')
    parser.add_argument('-n', '--dryRun', action='store_true')
    parser.add_argument('-j', '--jobs', type=int, default=1)
    parser.add_argument('-o', '--outDir')
    parser.add_argument('-t', '--testName')
    args = parser.parse_args()

    with open(args.configPath, "r") as f:
        environment = jinja2.Environment()
        template = environment.from_string(f.read())
        config, tests = yaml.safe_load_all(template.render(glob=glob))

    config['dryRun'] = args.dryRun
    config['jobs'] = args.jobs
    if args.outDir: config['outDir'] = args.outDir
    config['onlyRunTestName'] = args.testName

    config.setdefault('resolution', [600, 480])

    return config, tests


def mkdir(path):
    os.makedirs(path, exist_ok=True)

def extract(d:dict, k, default=None):
    v = d.get(k, default)
    if k in d: del d[k]
    return v

def alwaysList(x):
    if x is None: return []
    if isinstance(x, list): return x
    if isinstance(x, dict): return list(x.items())
    return [x]

def alwaysDict(x):
    if x is None: return {}
    if isinstance(x, dict): return x
    if isinstance(x, list):
        assert len(set(x)) == len(x)
        return {k:None for k in x}
    return {x: None}

def alwaysIterable(x):
    if x is None: return iter(())
    if isinstance(x, str): return iter((x,))
    try: return iter(x)
    except TypeError: return iter((x,))

def locateRelPath(path, root):
    if os.path.isabs(path):
        return path
    return os.path.join(root, path)

def GetConfigForCompletedRun(newConfig):
    if not os.path.exists(newConfig['outputLog']): return None
    with open(newConfig['outputLog'], "r") as log:
        lines = log.readlines()
        oldConfigYaml = ''.join(lines[lines.index("=========== CONFIG ===========\n")+1:lines.index("==============================\n")])
        return yaml.safe_load(oldConfigYaml)

import signal
import psutil

gotKeyboardInterrupt = False

def signal_handler(sig, frame):
    global gotKeyboardInterrupt
    gotKeyboardInterrupt = True
    current_process = psutil.Process()
    children = current_process.children(recursive=True)
    for child in children:
        signal.pthread_kill(child.pid, signal.SIGKILL)
    print('******* Jobs cancelled *******')
signal.signal(signal.SIGINT, signal_handler)

def RunTest(config):
    if os.path.exists(config['output']) and os.path.exists(config['outputLog']):
        with open(config['outputLog'], "r") as log:
            if "=========== TEST COMPLETE ===========" in log.read():
                if config == GetConfigForCompletedRun(config):
                    # print("Skipping test", config['name'], config['renderer'])
                    return
                else:
                    print("Config changed for", config['name'])
    print("Run test", config['name'], config['renderer'])
    cmd = ['python', '-u', 'run_test.py', yaml.dump(config)]

    with open(config['outputLog'], "w", 1) as log:
        try:
            subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, text=True)
        except Exception:
            pass
        if gotKeyboardInterrupt:
            print("Stopped", config['name'], config['renderer'])
            return
        print("\n=========== TEST COMPLETE ===========\n", file=log)
        if not os.path.exists(config['output']):
            copyfile("missing.png", config['output'])

def TestCanBeRunConcurrently(test:dict) -> bool:
    return test.get('runConcurrently', True)

def GenerateTests(config, tests):
    for name, test in tests.items():
        files = [locateRelPath(f, config['dataRootDir']) for f in alwaysList(extract(test, 'files'))]
        files = sorted(chain.from_iterable(map(glob, files)))
        dataType = extract(test, 'type')

        if extract(test, 'skip'):
            continue

        renderers:dict = alwaysDict(extract(test, 'renderers', config['rendererSets']['default']))
        for k in renderers.copy():
            if k in config['rendererSets']:
                del renderers[k]
                for r, v in alwaysDict(config['rendererSets'][k]).items():
                    renderers.setdefault(r, v)

        for ren, renConfig in renderers.items():
            if isinstance(renConfig, str):
                renConfig = {'variable': renConfig}

            renConfig = alwaysDict(renConfig)
            variableKeys = [k for k in renConfig if 'variable' in k.lower()]
            variables = [i for i in renConfig.items() if i[0] in variableKeys]
            for k,_ in variables:
                del renConfig[k]

            varCombos = [*product(*[[*zip(repeat(k), alwaysList(v))] for k,v in variables])]

            for combo in varCombos:
                fullName = f"{name}-{ren}"
                if len(varCombos) > 1:
                    fullName += '-'.join(['', *chain.from_iterable(combo)])
                fName = fullName + ".png"
                fName = fName.replace('/', '-')

                yield {
                    'name': name,
                    'outputDir': config['outDir'],
                    'outputName': fName,
                    'output': f"{config['outDir']}/{fName}",
                    'outputSession': f"{config['outDir']}/meta/{fName}.vs3",
                    'outputLog': f"{config['outDir']}/meta/{fName}.log",
                    'files': files,
                    'type': dataType,
                    'renderer': ren,
                    'dryRun': config['dryRun'],
                    'resolution': config['resolution'],
                    **dict(combo),
                    **renConfig,
                    **test,
                }


if __name__ == "__main__":
    config, testConfigs = ParseAruments()

    # print("config = ", end='')
    # pprint.PrettyPrinter(indent=4).pprint(config)

    mkdir(config['outDir'])
    mkdir(f"{config['outDir']}/meta")

    tests = GenerateTests(config, testConfigs)
    if config['onlyRunTestName']:
        tests = filter(lambda t: fnmatch.fnmatch(t['name'], config['onlyRunTestName']), tests)

    tests = list(tests)
    nJobs = config.get('jobs', 1)

    if nJobs > 1:
        concurrentTests = filter(TestCanBeRunConcurrently, tests)
        singleTests = filter(lambda t: not TestCanBeRunConcurrently(t), tests)
    else:
        singleTests = tests
        concurrentTests = []

    if concurrentTests:
        print(f"Running concurrent tests with {nJobs} jobs")
        with concurrent.futures.ThreadPoolExecutor(max_workers=nJobs) as executor:
            executor.map(RunTest, concurrentTests)

    if singleTests:
        print(f"Running single-threaded tests")
        for test in singleTests:
            RunTest(test)
