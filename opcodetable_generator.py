from itertools import repeat

unknownInstructionCallback = 'chip8_uic'

instructions = []
instructions += repeat(unknownInstructionCallback, 65536)

def addInstruction(code, method):
    code = code.replace('n', '_').replace('k', '_').replace('x', '_').replace('y', '_')
    if not '_' in code:
        instructions[int(code, 16)] = method
        #print("'%s' -> '%s'" % (code, method))
    else:
        for i in range(0, 16):
            hexCode = '%1x' % (i)
            newCode = hexCode.join(code.rsplit('_', 1))
            addInstruction(newCode, method)

with open('chip8_impl.h', 'r') as fChipImpl:
    wasCommentBegin = False
    wasCommentEnd = False
    instructionCode = ""
    method = ""
    for line in fChipImpl.readlines():
        if wasCommentBegin and len(line) > 4:
            instructionCode = line.rstrip('\n').rstrip('\r')[:line.find(" ")]
            wasCommentBegin = False
        if wasCommentEnd and len(line) > 4:
            method = line[line.find(" ")+1:line.find('(')]
            wasCommentEnd = False
        if line.find("/*") >= 0:
            wasCommentBegin = True
        elif line.find("*/") >= 0:
            wasCommentEnd = True
        
        if instructionCode != "" and method != "":
            addInstruction(instructionCode, method)
            instructionCode = ""
            method = ""

def print_array(arr):
    print('/* TODO: Correct syntax error at the end of the list ",}" */')
    print('static void (*opcode_table[%d]) (chip8_t*,opcode_params_t*) = {' % (len(arr)), end="")
    for method in arr:
        print('%s,' % method, end="")
    print('};', end="")
print_array(instructions)
