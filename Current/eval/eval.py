import subprocess
import os
from os import listdir, popen
from os.path import join


def output(directory, name, ans, end='\n'):
    print(directory, '\t', '%-24s' % name, '\t', ans, end=end)


allDirectory = listdir('testcases')

passed = total = 0

for directory in allDirectory:
    for fileName in listdir(join('testcases', directory)):
        name = fileName.replace('.cminus', '')
        codeFileName = join('testcases', directory, fileName)
        inputFileName = join('answers', directory, name + '.in')
        targetOutputFileName = join('answers', directory, name + '.out')

        try:
            with open(inputFileName, 'rb') as inputFile:
                inputContent = inputFile.read()
        except:
            inputContent = b""

        try:
            os.system(f'../../build/cminusfc {codeFileName} -mem2reg -S -o temp 2>/dev/null')
            os.system('clang temp.s -o temp -L. -lcminus_io 2>/dev/null')
            result = subprocess.run(['./temp'], input=inputContent, stdout=subprocess.PIPE, timeout=5)
            outputContent = result.stdout
            with open(targetOutputFileName, 'rb') as targetOutputFile:
                targetOutputFileContent = targetOutputFile.read()
            if (outputContent == targetOutputFileContent):
                # output(directory, name, '')
                passed += 1
            else:
                output(directory, name, '\033[31;1m wrong\033[0m' + '\t', end='')
                print('\t', outputContent)
        except:
            output(directory, name, '\033[34;1m error\033[0m')
        total += 1
        os.system('rm temp.s temp.out temp 2>/dev/null')

print(passed, '/', total)
