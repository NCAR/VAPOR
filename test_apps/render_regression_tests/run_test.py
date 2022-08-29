import os.path
import sys

configExample = """
output: out.png
files:
  - /Users/stasj/Work/data/DUKU/DUKU-3.2.0.alpha.nc
type: vdc
renderer: TwoDData
variable: ALBEDO
timestep: 0
"""

try:
    configText = sys.argv[1]
except IndexError:
    configText = configExample
    print("WARNING: Missing config")
dryRun = True if len(sys.argv) >= 3 and sys.argv[2] == '-n' else False

print("=========== CONFIG ===========")
print(configText)
print("==============================")
print()

import yaml
config:dict = yaml.safe_load(configText)
# print("config =", config)
dryRun = dryRun or config.get('dryRun', False)

if dryRun:
    exit(0)

sys.path.append("vapor_module_dir")
import vapor.session
ses = vapor.session.Session()

dataset = ses.OpenDataset(config['type'], config['files'])

if 'resolution' in config:
    ses.SetResolution(config['resolution'][0], config['resolution'][1])

print(dataset)
for n in [2,3]:
    print(f"\t {n}D Vars: {', '.join(dataset.GetDataVarNames(n))}")

if 'timestep' in config:
    print("Setting timestep to", config['timestep'])
    ses.SetTimestep(config['timestep'])

def GetRendererClass(typ:str):
    typ = typ.removesuffix("2D")
    typ = typ.removesuffix("3D")
    for Class in vapor.renderer.Renderer.__subclasses_rec__():
        if Class.VaporName == typ:
            return Class

ren = dataset.NewRenderer(GetRendererClass(config['renderer']))
if config['renderer'].endswith("2D"): ren.SetDimensions(2)
if config['renderer'].endswith("3D"): ren.SetDimensions(3)
print(ren)

for var in [k for k in config.keys() if 'variable' in k.lower()]:
    setter = f"Set{var[0].upper()+var[1:]}Name"
    varName = config[var]
    print(f"ren.{setter}({varName})")
    ren.__getattribute__(setter)(varName)

# for c in config.get('commands', []):

if 'IsoValues' in config: ren.SetIsoValues(config['IsoValues'])


ses.GetCamera().ViewAll()
if 'outputSession' in config:
    ses.Save(config['outputSession'])
ses.Render(config['output'])
