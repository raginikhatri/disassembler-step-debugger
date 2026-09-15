#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <executable>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd < 0)
    {
        perror("Could not open file");
        return 1;
    }

    Elf32_Ehdr header;

    ssize_t bytes_read = read(fd, &header, sizeof(header));

    if (bytes_read != sizeof(header))
    {
        printf("Could not read ELF header.\n");
        close(fd);
        return 1;
    }

    // Check ELF magic number 
    if (header.e_ident[EI_MAG0] != ELFMAG0 ||
        header.e_ident[EI_MAG1] != ELFMAG1 ||
        header.e_ident[EI_MAG2] != ELFMAG2 ||
        header.e_ident[EI_MAG3] != ELFMAG3)
    {
        printf("Not a valid ELF file.\n");
        close(fd);
        return 1;
    }

    // Check whether it is 32-bit 
    if (header.e_ident[EI_CLASS] != ELFCLASS32)
    {
        printf("This is not a 32-bit ELF file.\n");
        close(fd);
        return 1;
    }

    // Check whether it is x86 
    if (header.e_machine != EM_386)
    {
        printf("This is not an x86 (32-bit) executable.\n");
        close(fd);
        return 1;
    }

    printf("Valid x86 32-bit ELF file\n\n");

    printf("Entry point : 0x%x\n", header.e_entry);
    printf("Section table offset : 0x%x\n", header.e_shoff);
    printf("Number of sections : %d\n", header.e_shnum);
    printf("Section header size : %d bytes\n", header.e_shentsize);

    close(fd);


    /* Read Section Header Table */

Elf32_Shdr *section_headers;

section_headers = malloc(header.e_shnum * header.e_shentsize);

if (section_headers == NULL)
{
    printf("Memory allocation failed.\n");
    close(fd);
    return 1;
}

if (lseek(fd, header.e_shoff, SEEK_SET) == -1)
{
    perror("Could not find section headers");
    free(section_headers);
    close(fd);
    return 1;
}

ssize_t bytes_read = read(
    fd,
    section_headers,
    header.e_shnum * header.e_shentsize
);

if (bytes_read != header.e_shnum * header.e_shentsize)
{
    printf("Could not read section headers.\n");
    free(section_headers);
    close(fd);
    return 1;
}


//Read Section Name String Table 

Elf32_Shdr *string_table =
    &section_headers[header.e_shstrndx];

char *section_names = malloc(string_table->sh_size);

if (section_names == NULL)
{
    printf("Memory allocation failed.\n");
    free(section_headers);
    close(fd);
    return 1;
}

if (lseek(fd, string_table->sh_offset, SEEK_SET) == -1)
{
    perror("Could not find section name table");
    free(section_names);
    free(section_headers);
    close(fd);
    return 1;
}

bytes_read = read(
    fd,
    section_names,
    string_table->sh_size
);

if (bytes_read != string_table->sh_size)
{
    printf("Could not read section names.\n");
    free(section_names);
    free(section_headers);
    close(fd);
    return 1;
}


// Find .text 

Elf32_Shdr *text_section = NULL;

for (int i = 0; i < header.e_shnum; i++)
{
    char *section_name =
        section_names + section_headers[i].sh_name;

    printf("Section %d: %s\n", i, section_name);

    if (strcmp(section_name, ".text") == 0)
    {
        text_section = &section_headers[i];
        break;
    }
}

if (text_section == NULL)
{
    printf(".text section not found.\n");
}
else
{
    printf("\n.text section found!\n");
    printf("Address : 0x%x\n", text_section->sh_addr);
    printf("Offset  : 0x%x\n", text_section->sh_offset);
    printf("Size    : %u bytes\n", text_section->sh_size);
}
    
    return 0;
}
