# woody-woodpacker

### simple elf64 binary encryptor


#### building:
`make`

#### usage:
`usage: ./woody_woodpacker [-d -k key] binary`

`-k` option to specify and encryption key

`-d` option to decrypt an encrypted binary

#### logic:
   - find `.text` section, get file offset and size
   - find `LOAD` segment which contains the `.text` section
   - go to end of segment and check if there are enough null bytes to insert decryptor
   - if yes, encrypt `.text` section, insert decryptor at the end of segment, increase segment size
   - save entry point, start address and size of `.text` section in decryptor
   - change entry point in elf header to point to decryptor

#### encryption:
random 64 bit key from `/dev/urandom`

for each byte of data:
- simple `xor` with lower 8 bits of key, 
- rotate key to the right `>>` by 1


#### sample:

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


#### comparison of hexdumps: before and after encryption
[![intra/woody_hexdump_vimdiff.png](intra/woody_hexdump_vimdiff.png "intra/woody_hexdump_vimdiff.png")](intra/woody_hexdump_vimdiff.png "intra/woody_hexdump_vimdiff.png")


#### intra
[![intra/grade.png](intra/grade.png "intra/grade.png")](intra/grade.png "intra/grade.png")
