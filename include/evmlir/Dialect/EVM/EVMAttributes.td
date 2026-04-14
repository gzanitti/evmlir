include "mlir/IR/EnumAttr.td"
include "mlir/IR/BuiltinAttributeInterfaces.td"

def EVM_VisibilityAttr : I32EnumAttr<"Visibility",
    "EVM function visibility",
    [
        I32EnumAttrCase<"External", 0, "external">,
        I32EnumAttrCase<"Internal", 1, "internal">
    ]> {
    let cppNamespace = "::evmlir::evm";
    let genSpecializedAttr = 0;
}

def EVM_FunctionKindAttr : I32EnumAttr<"FunctionKind",
    "EVM function kind",
    [
        I32EnumAttrCase<"Function",    0, "function">,
        I32EnumAttrCase<"Constructor", 1, "constructor">,
        I32EnumAttrCase<"Fallback",    2, "fallback">,
    ]> {
    let cppNamespace = "::evmlir::evm";
    let genSpecializedAttr = 0;
}