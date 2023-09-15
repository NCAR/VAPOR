import xml.etree.ElementTree as ET
import functools
from . import config


class CPPYYDoxygenWrapperMeta(type):
    __doxygenRootPath = config.GetDoxygenRoot()

    @classmethod
    @functools.cache
    def __GetDoxgenRoot(cls) -> ET.Element:
        if not cls.__doxygenRootPath:
            print("WARNING DOxygen not found")
            return None
        root = ET.parse(cls.__doxygenRootPath+'/index.xml').getroot()
        return root

    @classmethod
    @functools.cache
    def __GetClassIndexNode(cls, name:str) -> ET.Element:
        root: ET.Element = cls.__GetDoxgenRoot()
        if not root: return None
        return root.find(f"./compound/name[.='{name}']/..")

    @classmethod
    @functools.cache
    def __GetClassRoot(cls, name:str) -> ET.Element:
        classIndexNode = cls.__GetClassIndexNode(name)
        if not classIndexNode: return None
        relPath = classIndexNode.get('refid') + '.xml'
        absPath = f"{cls.__doxygenRootPath}/{relPath}"
        try:
            root = ET.parse(absPath).getroot()
        except IOError:
            return  None
        return root

    @classmethod
    def __GetMemberNodes(cls, className:str, memberName:str) -> list[ET.Element]:
        """Returns list for overloaded functions"""
        root: ET.Element = cls.__GetClassRoot(className)
        if not root: return None
        return root.findall(f"./compounddef/sectiondef/memberdef/name[.='{memberName}']/..")

    @classmethod
    def __ParseDescriptionParameterList(cls, plist: ET.Element) -> str:
        return " ".join([t.strip() for t in plist.itertext() if t.strip()]).replace("\n","")

    @classmethod
    def __RenderParameterListSection(cls, title:str, kind:str, allLists:list[ET.Element]):
        ret = ""
        kindLists = [pl for pl in allLists if pl.get("kind") == kind]
        if kindLists:
            ret += f"\n{title}\n"
            for l in kindLists:
                ret += "    " + cls.__ParseDescriptionParameterList(l)
        return ret

    @classmethod
    def __ParseDescriptionParagraph(cls, para: ET.Element, parameterLists: list[ET.Element] = []) -> str:
        if para.tag == "parameterlist" or (para.tag == "simplesect" and para.get("kind")):
            parameterLists.append(para)
            return ""

        text = ""
        if para.text:
            text += para.text.strip()

        for e in para:
            ret = cls.__ParseDescriptionParagraph(e, parameterLists)
            if ret.strip():
                text += " " + ret

        if para.tail and para.tail.strip():
            text += " " + para.tail.strip()
        return text

    @classmethod
    def __ParseMemberDetailedDescription(cls, member: ET.Element) -> str:
        ddNode = member.find('detaileddescription')
        paragraphs = ddNode.findall('para')
        parameterLists: list[ET.Element] = []
        parsedParas = [cls.__ParseDescriptionParagraph(p, parameterLists) for p in paragraphs]
        ret = "\n".join(["    "+p for p in parsedParas if p])

        if parameterLists:
            ret += cls.__RenderParameterListSection("Parameters", "param", parameterLists)
            ret += cls.__RenderParameterListSection("Returns", "retval", parameterLists)
            ret += cls.__RenderParameterListSection("See Also", "see", parameterLists)

        return ret

    @classmethod
    def __CleanFunctionDefinition(cls, ds: str) -> str:
        ds = ds.removeprefix("virtual ")
        ds = ds.removesuffix(" const")
        return ds

    @classmethod
    def __GetDocumentationForMemberNode(cls, node: ET.Element) -> str:
        if node.get("kind") == "function":
            return cls.__GetDocumentationForFunctionNode(node)
        elif node.get("kind") == "variable":
            return cls.__GetDocumentationForVariableNode(node)

    @classmethod
    def __GetDocumentationForFunctionNode(cls, node: ET.Element) -> str:
        name = node.find('name').text
        definition = node.find('definition').text
        args = node.find('argsstring').text
        fullDefinition = cls.__CleanFunctionDefinition(definition+args)
        detailedDescription = cls.__ParseMemberDetailedDescription(node)

        return f"""
{fullDefinition}
{detailedDescription}
""".strip()

    @classmethod
    def __GetDocumentationForVariableNode(cls, node: ET.Element) -> str:
        briefdescription = cls.__ParseClassDescription(node.find('briefdescription'))
        detaileddescription = cls.__ParseClassDescription(node.find('detaileddescription'))
        return f"""
{briefdescription}
{detaileddescription}
""".strip()

    @classmethod
    def __GetFunctionDocumentation(cls, pCls:type, func) -> str:
        name = getattr(func, "__doxygen_name__",func.__name__)
        # if hasattr(func, "__doxygen_name__"):

        nodes = cls.__GetMemberNodes(pCls.__cpp_name__, name)
        if not nodes:
            # DOxygen does not inherit documentation so base classes need to be recursively queried
            if pCls.__base__ and cls.__GetClassRoot(pCls.__base__.__cpp_name__):
                return cls.__GetFunctionDocumentation(pCls.__base__, func)
            return func.__doc__
        return "\n\n".join([cls.__GetDocumentationForMemberNode(node) for node in nodes])

    @classmethod
    def __ParseClassDescription(cls, ddNode: ET.Element) -> str:
        paragraphs = ddNode.findall('para')
        parameterLists: list[ET.Element] = []
        parsedParas = [cls.__ParseDescriptionParagraph(p, parameterLists) for p in paragraphs]
        ret = "\n".join(["    " + p for p in parsedParas if p])

        return ret

    @classmethod
    def __GetClassDocumentation(cls, pCls) -> str:
        className = pCls.__cpp_name__
        root: ET.Element = cls.__GetClassRoot(className)
        if not root: return None
        root = root.find('compounddef')
        if not root: return None
        briefdescription    = cls.__ParseClassDescription(root.find('briefdescription'))
        detaileddescription = cls.__ParseClassDescription(root.find('detaileddescription'))
        return f"""
Wraps {className}
{briefdescription}
{detaileddescription}
""".strip()

    @classmethod
    def _MakeFunctionWrapper(cls, pCls, func):
        @functools.wraps(func)
        def f(self, *args, **kwargs):
            ret = func(self, *args, **kwargs)
            return ret

        f.__doc__ = cls.__GetFunctionDocumentation(pCls, func)

        return f

    def __new__(cls, clsname, bases, clsdict:dict, wrap:type=None):
        if wrap:
            clsdict['__doc__'] = cls.__GetClassDocumentation(wrap)
        return super(CPPYYDoxygenWrapperMeta, cls).__new__(cls, clsname, bases, clsdict)
