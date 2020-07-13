import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input')
    parser.add_argument('symbol')
    args = parser.parse_args()

    with open(args.input, 'rb') as f:
        print('static const char {}[] = {{'.format(args.symbol))
        while True:
            bytes = f.read(12)
            if not bytes:
                break
            print('   {0}'.format(''.join('0x{:02x}, '.format(x) for x in bytes)))
        print('};')

if __name__ == "__main__":
    main()
