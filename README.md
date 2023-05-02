# woody-woodpacker

simple elf64 binary encryptor


## building:
`make`

## usage:
`usage: ./woody_woodpacker [-d -k key] binary`

`-k` option to specify and encryption key

`-d` option to decrypt an encrypted binary

## logic:
   - find `.text` section, get file offset and size
   - find `LOAD` segment which contains the `.text` section
   - go to end of segment and check if there are enough null bytes to insert decryptor
   - if yes, encrypt `.text` section, insert decryptor at the end of segment, increase segment size
   - save entry point, start address and size of `.text` section in decryptor
   - change entry point in elf header to point to decryptor

## encryption:
random 64 bit key from `/dev/urandom`

for each byte of data:
- simple `xor` with lower 8 bits of key, 
- rotate key to the right `>>` by 1


## sample:
```
root@debian:/home/user/woody-woodpacker# cat hello.c 
#include <stdio.h>
int main()
{
	puts("Hello World!");
	return 0;
}
root@debian:/home/user/woody-woodpacker# gcc hello.c -o hello
root@debian:/home/user/woody-woodpacker# ./woody_woodpacker hello
encryption key = 0x334f9db9c8c89c93
root@debian:/home/user/woody-woodpacker# ./woody
..WOODY..
Hello World!
root@debian:/home/user/woody-woodpacker# 
```

## comparison of hexdumps: before and after encryption

`radiff2 -x hello woody`

```sh
  offset     0 1 2 3 4 5 6 7 8 9 A B C D E F 0123456789ABCDEF    0 1 2 3 4 5 6 7 8 9 A B C D E F 0123456789ABCDEF
0x00000000  7f454c46020101000000000000000000 .ELF............   7f454c46020101000000000000000000 .ELF............
0x00000010! 02003e00010000004010400000000000 ..>.....@.@.....   02003e00010000004911400000000000 ..>.....I.@.....
0x00000020  40000000000000001054000000000000 @........T......   40000000000000001054000000000000 @........T......
0x00000030  00000000400038000d0040001f001e00 ....@.8...@.....   00000000400038000d0040001f001e00 ....@.8...@.....
...
0x000000e0! 00100000000000000100000005000000 ................   00100000000000000100000007000000 ................
0x000000f0  00100000000000000010400000000000 ..........@.....   00100000000000000010400000000000 ..........@.....
0x00000100! 00104000000000004901000000000000 ..@.....I.......   0010400000000000b601000000000000 ..@.............
0x00000110! 49010000000000000010000000000000 I...............   b6010000000000000010000000000000 ................
0x00000120  01000000040000000020000000000000 ......... ......   01000000040000000020000000000000 ......... ......
0x00000130  00204000000000000020400000000000 . @...... @.....   00204000000000000020400000000000 . @...... @.....
...
0x00001040! f30f1efa31ed4989d15e4889e24883e4 ....1.I..^H..H..   60463a68f8093bb04d106f9a6b0ca175 `F:h..;.M.o.k..u
0x00001050! f050544531c031c948c7c726114000ff .PTE1.1.H..&.@..   3834665cbd8612588023b51f8d8ee78c 84f\...X.#......
0x00001060! 15732f0000f4662e0f1f840000000000 .s/...f.........   acaf41b7db19101592d163f3f97c3e9f ..A.......c..|>.
0x00001070! f30f1efac3662e0f1f84000000000090 .....f..........   bca8cd93f7fce3692c1dcc6633994cb6 .......i,..f3.L.
0x00001080! b810404000483d104040007413b80000 ..@@.H=.@@.t....   2b5964d2c9ac4f29dc0e27679afc2291 +Yd...O)..'g..".
0x00001090! 00004885c07409bf10404000ffe06690 ..H..t...@@...f.   c8647a9c4c322a2ed8a43239632e81e3 .dz.L2*...29c...
0x000010a0! c366662e0f1f8400000000000f1f4000 .ff...........@.   7aba0899d4f2f23b9dcee7f3f6637e9f z......;.....c~.
0x000010b0! be104040004881ee104040004889f048 ..@@.H...@@.H..H   f1b7932934d24c8823d98c667b10bc6e ...)4.L.#..f{..n
0x000010c0! c1ee3f48c1f8034801c648d1fe7411b8 ..?H...H..H..t..   52a71bda081c71719d886fc277303329 R.....qq..o.w03)
0x000010d0! 000000004885c07407bf10404000ffe0 ....H..t...@@...   c8643219c4c3e3e5cf5b6279dcce1893 .d2......[by....
0x000010e0! c366662e0f1f8400000000000f1f4000 .ff...........@.   7aba0899d4f2f23b9dcee7f3f6637e9f z......;.....c~.
0x000010f0! f30f1efa803d112f0000007513554889 .....=./...u.UH.   bca8cd93b4a7dc493399cc1320cc04af .......I3... ...
0x00001100! e5e87affffffc605ff2e0000015dc390 ..z..........]..   76a15e6d361bb43c636027138819e101 v.^m6..<c`'.....
0x00001110! c366662e0f1f8400000000000f1f4000 .ff...........@.   0b0254378359a791c8e4723993d1a773 ..T7.Y....r9...s
0x00001120! f30f1efaeb8a554889e5bf10204000e8 ......UH.... @..   4ad3704d30672373142b58e3d93c3e77 J.pM0g#s.+X..<>w
0x00001130! fcfeffffb8000000005dc300f30f1efa .........]......   b3592c968c9acd6633c40f00f30f1efa .Y,....f3.......
0x00001140! 4883ec084883c408c300000000000000 H...H...........   4883ec084883c408c331c099b20affc0 H...H....1......
0x00001150! 00000000000000000000000000000000 ................   89c7488d352b0000000f05488d46c548 ..H.5+.....H.F.H
0x00001160! 00000000000000000000000000000000 ................   2b462a4889c74803460a5048037e1248 +F*H..H.F.PH.~.H
0x00001170! 00000000000000000000000000000000 ................   8b4e1a488b4622300748d1c848ffc7e2 .N.H.F"0.H..H...
0x00001180! 00000000000000000000000000000000 ................   f6f7e1c32e2e574f4f44592e2e0a4010 ......WOODY...@.
0x00001190! 00000000000000000000000000000000 ................   4000000000004010400000000000fb00 @.....@.@.......
0x000011a0! 00000000000000000000000000000000 ................   000000000000939cc8c8b99d4f334911 ............O3I.
0x000011b0! 00000000000000000000000000000000 ................   40000000000000000000000000000000 @...............
0x000011c0  00000000000000000000000000000000 ................   00000000000000000000000000000000 ................
0x000011d0  00000000000000000000000000000000 ................   00000000000000000000000000000000 ................
```
