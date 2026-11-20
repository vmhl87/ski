import sys

def tokenize(line):
    tokens = []
    i = 0
    n = len(line)
    while i < n:
        c = line[i]
        if c.isspace():
            i += 1
            continue
        if c in "SKI()":
            tokens.append(c)
            i += 1
        elif c == '[':
            j = line.index(']', i)  # find matching closing bracket
            tokens.append(line[i:j+1])
            i = j + 1
        else:
            raise ValueError(f"Unexpected character {c!r} at position {i}")
    return tokens

def main():
    line = sys.stdin.readline().rstrip('\n')
    tokens = tokenize(line)
    print(' '.join(tokens))

if __name__ == '__main__':
    main()
