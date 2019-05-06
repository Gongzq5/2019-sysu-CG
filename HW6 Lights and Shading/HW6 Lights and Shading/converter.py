# GLSL Converter

if __name__ == '__main__':
    const_char_shader = []
    with open('hw4.vs') as f:
        lines = f.readlines()
        for line in lines:
            print('\"' + line.replace('\n', '') + '\\n\"')
    with open('hw4.fs') as f:
        lines = f.readlines()
        for line in lines:
            print('\"' + line.replace('\n', '') + '\\n\"')