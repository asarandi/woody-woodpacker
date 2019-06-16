#include <elf.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

char *file_get_contents(char *f, ssize_t *size)
{
    int         fd;
    ssize_t     ret;
    struct stat st;
    char        *data;

    if ((fd = open(f, O_RDONLY)) == -1)
        return NULL;        
    data = NULL;
    if (fstat(fd, &st) == 0)
        data = malloc(st.st_size);
    ret = -1;
    if (data)
        ret = read(fd, data, st.st_size);
    close(fd);    
    *size = st.st_size;
    if (ret == st.st_size)  /* maybe include a check for minimum valid file size ? */
        return data;
    if (data)
        free(data);
    return NULL;
}

int file_put_contents(char *f, void *d, ssize_t s, int m)
{
    int     fd;
    ssize_t ret;

    fd = open(f, O_WRONLY | O_CREAT | O_TRUNC, m);
    if (fd == -1)
        return 0;
    ret = write(fd, d, s);
    close(fd);
    return ret == s ? 1:0;
}

Elf64_Phdr  *get_code_segment(void *data)
{
    Elf64_Ehdr  *ehdr = data;
    Elf64_Phdr  *phdr = data + ehdr->e_phoff;

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        if ((phdr->p_vaddr == phdr->p_paddr) &&
            (phdr->p_memsz == phdr->p_filesz) &&
            (phdr->p_type == 1) &&
            (phdr->p_flags == 5) &&
            (ehdr->e_entry >= phdr->p_vaddr) &&
            (ehdr->e_entry < phdr->p_vaddr + phdr->p_memsz))
            return phdr;            
        phdr++;
    }
    return NULL;
}

ssize_t get_cavity_size(void *data, Elf64_Phdr *phdr)
{
    void    *ptr_start;
    void    *ptr_end;
    ssize_t cavity_size;

    ptr_start = data + phdr->p_offset + phdr->p_filesz;
    ptr_end = data + phdr->p_offset;
    while (ptr_end < ptr_start)
        ptr_end += phdr->p_align;
    cavity_size = ptr_end - ptr_start;
    while (ptr_start < ptr_end)
    {
        if (*(unsigned char *)ptr_start)
            return 0;
        ptr_start++;            
    }
    return cavity_size;
}

int is_valid_elf64(void *data)
{
    Elf64_Ehdr  *ehdr = data;
   
    const unsigned char elf64_valid_header[] = {
      0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x02, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00
    };

    if (memcmp(data, elf64_valid_header, sizeof(elf64_valid_header)))
        return 0;
    if (ehdr->e_phentsize != sizeof(Elf64_Phdr)) /* XXX wtf */
        return 0;
    if (ehdr->e_shentsize != sizeof(Elf64_Shdr)) /* XXX wtf */
        return 0;
    return 1;
}

int free_msg_quit(void *p, char *s)
{
    if (p)
        free(p);
    printf("%s\n", s);
    return 0;
}

/*

bits 64
default rel
global _start
section .text
_start:
    push    rdi
    push    rsi
    push    rdx
    mov     rdi, [rel address]
    mov     rsi, [rel size]    
    mov     rdx, [rel key]
    add     rsi, rdi
.loop:    
    cmp     rdi, rsi
    jge     .done
    xor     byte [rdi], dl
    inc     rdi
    ror     rdx, 8
    jmp     .loop
.done:
    pop     rdx
    pop     rsi
    pop     rdi
    jmp     [rel ep]

    ep      dq 0x1111111122222222
    address dq 0x3333333344444444
    size    dq 0x5555555566666666
    key     dq 0x7777777788888888

$ nasm -f elf64 decryptor.s 
$ ld decryptor.o -o decryptor
$ xxd -c 8 -i -s 0x1000 -l 84 decryptor

unsigned char decryptor[] = {
  0x57, 0x56, 0x52, 0x48, 0x8b, 0x3d, 0x32, 0x00,
  0x00, 0x00, 0x48, 0x8b, 0x35, 0x33, 0x00, 0x00,
  0x00, 0x48, 0x8b, 0x15, 0x34, 0x00, 0x00, 0x00,
  0x48, 0x01, 0xfe, 0x48, 0x39, 0xf7, 0x7d, 0x0b,
  0x30, 0x17, 0x48, 0xff, 0xc7, 0x48, 0xc1, 0xca,
  0x08, 0xeb, 0xf0, 0x5a, 0x5e, 0x5f, 0xff, 0x25,
  0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22,
  0x11, 0x11, 0x11, 0x11, 0x44, 0x44, 0x44, 0x44,
  0x33, 0x33, 0x33, 0x33, 0x66, 0x66, 0x66, 0x66,
  0x55, 0x55, 0x55, 0x55, 0x88, 0x88, 0x88, 0x88,
  0x77, 0x77, 0x77, 0x77
};
unsigned int decryptor_len = 84;

*/



const unsigned char   decryptor_code[52] = {
      0x57, 0x56, 0x52, 0x48, 0x8b, 0x3d, 0x32, 0x00,
      0x00, 0x00, 0x48, 0x8b, 0x35, 0x33, 0x00, 0x00,
      0x00, 0x48, 0x8b, 0x15, 0x34, 0x00, 0x00, 0x00,
      0x48, 0x01, 0xfe, 0x48, 0x39, 0xf7, 0x7d, 0x0b,
      0x30, 0x17, 0x48, 0xff, 0xc7, 0x48, 0xc1, 0xca,
      0x08, 0xeb, 0xf0, 0x5a, 0x5e, 0x5f, 0xff, 0x25,
      0x00, 0x00, 0x00, 0x00 };

typedef struct      s_decryptor_values
{
    uint64_t        ep;
    uint64_t        address;
    uint64_t        size;
    uint64_t        key;
}                   t_dv;

#define DECRYPTOR_SIZE (sizeof(decryptor_code) + sizeof(struct s_decryptor_values))     /* should be 52 + 32 = 84 bytes total */

uint64_t encrypt(void *data, Elf64_Phdr *phdr)  /* return 64 bit key */
{
    void    *ptr_start;
    void    *ptr_end;
    uint64_t key, key_copy;
    struct timespec tp;

    ptr_start = data + phdr->p_offset;
    ptr_end = data + phdr->p_offset + phdr->p_filesz;
    (void)clock_gettime(CLOCK_BOOTTIME, &tp);
    srand(tp.tv_sec ^ tp.tv_nsec);  /* try to figure out the key from file creation time */
    key = rand();
    key <<= 32;
    key += rand();
    key_copy = key;
    while (ptr_start < ptr_end)
    {
        *(unsigned char *)ptr_start ^= key & 0xff;
        key = (key >> 8) | ((key & 0xff) << 56);
        ptr_start++;            
    }
    return key_copy;
}

void insert(void *data, Elf64_Phdr *phdr)
{
    Elf64_Ehdr  *ehdr = data;
    t_dv        dv;
    void        *ptr;

    dv.ep = ehdr->e_entry;    
    ehdr->e_entry = phdr->p_vaddr + phdr->p_memsz;
    dv.address = phdr->p_vaddr;
    dv.size = phdr->p_memsz;
    dv.key = encrypt(data, phdr);
    ptr = data + phdr->p_offset + phdr->p_filesz;
    phdr->p_memsz += DECRYPTOR_SIZE;
    phdr->p_filesz += DECRYPTOR_SIZE;
    phdr->p_flags |= PF_X | PF_W | PF_R;
    memcpy(ptr, decryptor_code, sizeof(decryptor_code));
    memcpy(ptr + sizeof(decryptor_code), &dv, sizeof(t_dv));
}

int main(int ac, char **av)
{
    char        *data;
    Elf64_Phdr  *phdr;
    ssize_t     fs;

    if (ac != 2)
        return free_msg_quit(0, "input file name missing");
    fs = 0;        
    if (!(data = file_get_contents(av[1], &fs)))
        return free_msg_quit(0, "failed to read input file");
    if (!is_valid_elf64(data))
        return free_msg_quit(data, "file not valid");
    if (!(phdr = get_code_segment(data)))
        return free_msg_quit(data, "could not find code segment");
    if (get_cavity_size(data, phdr) < DECRYPTOR_SIZE)
        return free_msg_quit(data, "could not find suitable cavity");
    insert(data, phdr);
    if (!file_put_contents("woody", data, fs, 0755))
        printf("could not write output file");
    free(data);
    return 0;
}
