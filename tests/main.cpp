#include <string>
#include <ShaderTree/Test_ShaderTree.h>

int main(int argc, char** argv) {
    if (argc > 1) {
        printf("Exec test : %s\n", argv[1]);
        return Test_ShaderTree(argv[1]) ? 0 : 1;
    }
    return Test_ShaderTree("Test_ShaderNode_GetShaderSection_14") ? 0 : 1;
}