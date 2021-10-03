import re

inFile = open('syntax_analyzer_raw.y', 'r')
inContent = inFile.read()

lines = inContent.split('\n')
for i in range(len(lines)):
    matched = re.match(r'(\w+) : (.*);', lines[i])
    if matched:
        name = matched.group(1)
        expressions = matched.group(2).split(' | ')
        if name != 'program':
            for j in range(len(expressions)):
                num = len(re.findall('\w+', expressions[j]))
                dollars = ''.join([', $' + str(i) for i in range(1, num + 1)])
                expressions[j] += ' {{$$ = node("{0}", {1}{2});}}'.format(name.replace('_', '-'), str(num), dollars)
            lines[i] = '{0} : {1};'.format(name, '\n    | '.join(expressions))

outContent = '\n'.join(lines)
outFile = open('syntax_analyzer.y', 'w')
outFile.write(outContent)
