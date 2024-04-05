#include <ShaderTree/Test_ShaderTree.h>
#include <ShaderTree/parsing/Test_ParsingIncludeCode.h>
#include <ShaderTree/parsing/Test_ParsingSlotCode.h>
#include <ShaderTree/parsing/Test_ParsingStageParam.h>
#include <ShaderTree/parsing/Test_ParsingUniforms.h>
#include <ShaderTree/parsing/Test_ParsingUtils.h>
#include <ShaderTree/parsing/Test_SectionCode.h>
#include <ShaderTree/shader/Test_ShaderNode.h>

#include <ShaderTree/ShaderTree.h>
#include <ShaderTree/shader/ShaderNode.h>

////////////////////////////////////////////////////////////////////////////
//// ConnectSlots //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool Test_ShaderTree_Connect_Slots_0() {
    auto treePtr = ShaderTree::Create();

    ////////////// OUTPUT /////////////////////

    auto code_output =
        u8R"(
@FRAGMENT
[SLOT:OUT[void:SDF_BLOCK
float outSDF() {
    return 1.0;
}
]]
[SLOT:OUT[void:PATH_BLOCK
vec3 outPath(vec3 p) {
    return 1.0;
}
]]
)";
    auto output_node_ptr = treePtr->AddOrUpdateNodeFromCode("output", code_output).lock();
    if (output_node_ptr == nullptr)
        return false;
    output_node_ptr->dumpToScreen();
    if (!output_node_ptr->IsSlotCodeExist("FRAGMENT", "OUT", "SDF_BLOCK"))
        return false;
    if (!output_node_ptr->IsSlotCodeExist("FRAGMENT", "OUT", "PATH_BLOCK"))
        return false;

    ////////////// INPUT //////////////////////

    auto code_input =
        u8R"(
@FRAGMENT
[SLOT:IN[void:SDF_BLOCK
]]
[SLOT:IN[void:PATH_BLOCK
]]
)";
    auto input_node_ptr = treePtr->AddOrUpdateNodeFromCode("input", code_input).lock();
    if (input_node_ptr == nullptr)
        return false;
    input_node_ptr->dumpToScreen();
    if (!input_node_ptr->IsSlotCodeExist("FRAGMENT", "IN", "SDF_BLOCK"))
        return false;
    if (!input_node_ptr->IsSlotCodeExist("FRAGMENT", "IN", "PATH_BLOCK"))
        return false;
    if (!treePtr->ConnectSlots("output", "input"))
        return false;
    const auto& program_code = input_node_ptr->GetProgramCode();
    if (!program_code.IsOk())
        return false;
    if (!program_code.IsShaderStageNameExist("FRAGMENT"))
        return false;
    const auto& frag_code = program_code.GetShaderStageCode("FRAGMENT");
    if (frag_code.GetCode() != "float outSDF() {\n    return 1.0;\n}\nvec3 outPath(vec3 p) {\n    return 1.0;\n}\n")
        return false;
    return true;
}


bool Test_ShaderTree_Connect_Slots_1() {
    auto treePtr = ShaderTree::Create();

    ////////////// OUTPUT /////////////////////

    auto code_output =
        u8R"(
@SDF
[SLOT:OUT[void:SDF_BLOCK
float outSDF() {
    return 1.0;
}
]]
@PATH
[SLOT:OUT[void:PATH_BLOCK
vec3 outPath(vec3 p) {
    return 1.0;
}
]]
)";
    auto output_node_ptr = treePtr->CreateNode("output");
    if (output_node_ptr == nullptr)
        return false;
    output_node_ptr->AddLegalStage("SDF");
    output_node_ptr->AddLegalStage("PATH");
    output_node_ptr = treePtr->AddOrUpdateNodeFromCode("output", code_output).lock();
    if (output_node_ptr == nullptr)
        return false;
    output_node_ptr->dumpToScreen();
    if (!output_node_ptr->IsSlotCodeExist("SDF", "OUT", "SDF_BLOCK"))
        return false;
    if (!output_node_ptr->IsSlotCodeExist("PATH", "OUT", "PATH_BLOCK"))
        return false;

    ////////////// INPUT //////////////////////

    auto code_input =
        u8R"(
@FRAGMENT
[SLOT:IN[void:SDF_BLOCK
]]
[SLOT:IN[void:PATH_BLOCK
]]
)";
    auto input_node_ptr = treePtr->AddOrUpdateNodeFromCode("input", code_input).lock();
    if (input_node_ptr == nullptr)
        return false;
    input_node_ptr->dumpToScreen();
    if (!input_node_ptr->IsSlotCodeExist("FRAGMENT", "IN", "SDF_BLOCK"))
        return false;
    if (!input_node_ptr->IsSlotCodeExist("FRAGMENT", "IN", "PATH_BLOCK"))
        return false;
    if (!treePtr->ConnectSlots("output", "SDF", "SDF_BLOCK", "input", "FRAGMENT", "SDF_BLOCK"))
        return false;
    if (!treePtr->ConnectSlots("output", "PATH", "PATH_BLOCK", "input", "FRAGMENT", "PATH_BLOCK"))
        return false;
    const auto& program_code = input_node_ptr->GetProgramCode();
    if (!program_code.IsOk())
        return false;
    if (!program_code.IsShaderStageNameExist("FRAGMENT"))
        return false;
    const auto& frag_code = program_code.GetShaderStageCode("FRAGMENT");
    if (frag_code.GetCode() != "float outSDF() {\n    return 1.0;\n}\nvec3 outPath(vec3 p) {\n    return 1.0;\n}\n")
        return false;

    return true;
}

bool Test_ShaderTree_Connect_Slots_2() {
    auto treePtr = ShaderTree::Create();

    ////////////// OUTPUT /////////////////////

    auto code_output =
        u8R"(
@SDF
[SLOT:OUT[void:SDF_BLOCK
float outSDF() {
    return 1.0;
}
]]
@PATH
[SLOT:OUT[void:PATH_BLOCK
vec3 outPath(vec3 p) {
    return 1.0;
}
]]
)";
    auto output_node_ptr = treePtr->CreateNode("output");
    if (output_node_ptr == nullptr)
        return false;
    output_node_ptr->AddLegalStage("SDF");
    output_node_ptr->AddLegalStage("PATH");
    output_node_ptr = treePtr->AddOrUpdateNodeFromCode("output", code_output).lock();
    if (output_node_ptr == nullptr)
        return false;
    output_node_ptr->dumpToScreen();
    if (!output_node_ptr->IsSlotCodeExist("SDF", "OUT", "SDF_BLOCK"))
        return false;
    if (!output_node_ptr->IsSlotCodeExist("PATH", "OUT", "PATH_BLOCK"))
        return false;

    ////////////// INPUT //////////////////////

    auto code_input =
        u8R"(
@FRAGMENT
[SLOT:IN[void:SDF_BLOCK
]]
[SLOT:IN[void:PATH_BLOCK
]]
)";
    auto input_node_ptr = treePtr->AddOrUpdateNodeFromCode("input", code_input).lock();
    if (input_node_ptr == nullptr)
        return false;
    input_node_ptr->dumpToScreen();
    if (!input_node_ptr->IsSlotCodeExist("FRAGMENT", "IN", "SDF_BLOCK"))
        return false;
    if (!input_node_ptr->IsSlotCodeExist("FRAGMENT", "IN", "PATH_BLOCK"))
        return false;
    if (!treePtr->ConnectSlots("output", "SDF", "SDF_BLOCK", "input", "FRAGMENT", "SDF_BLOCK"))
        return false;
    if (!treePtr->ConnectSlots("output", "PATH", "PATH_BLOCK", "input", "FRAGMENT", "PATH_BLOCK"))
        return false;
    const auto& program_code = input_node_ptr->GetProgramCode();
    if (!program_code.IsOk())
        return false;
    if (!program_code.IsShaderStageNameExist("FRAGMENT"))
        return false;
    const auto& frag_code = program_code.GetShaderStageCode("FRAGMENT");
    if (frag_code.GetCode() != "float outSDF() {\n    return 1.0;\n}\nvec3 outPath(vec3 p) {\n    return 1.0;\n}\n")
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////
//// ENTRY POINT ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v)            \
    if (vTest == std::string(#v)) \
    return v()

#define IfTestCollectionExist(v, str)        \
    if (vTest.find(#v) != std::string::npos) \
    return v(str)

bool Test_ShaderTree(const std::string& vTest) {
    IfTestExist(Test_ShaderTree_Connect_Slots_0);
    else IfTestExist(Test_ShaderTree_Connect_Slots_1);
    else IfTestExist(Test_ShaderTree_Connect_Slots_2);
    else IfTestCollectionExist(Test_ParsingUtils, vTest);
    else IfTestCollectionExist(Test_SectionCode, vTest);
    else IfTestCollectionExist(Test_ParsingUniforms, vTest);
    else IfTestCollectionExist(Test_ParsingSlotCode, vTest);
    else IfTestCollectionExist(Test_ParsingIncludeCode, vTest);
    else IfTestCollectionExist(Test_ParsingStageParam, vTest);
    else IfTestCollectionExist(Test_ShaderNode, vTest);
    return true;
}