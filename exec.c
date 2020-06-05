#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"
#include "page.h"

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off, firstindex;
  uint numtextpage;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  uint vaddr;
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  struct sechdr sh, strtab;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();
  char names[512];
  uint textaddr = 0;
  uint textsz = 0;

  cprintf("Entering exec for prog %s\n", path);

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // skip allocating the initial page
  sz = PGSIZE;

  // load string table header into strtab
  if (readi(ip, (char *)&strtab, elf.shoff + elf.shstrndx * sizeof(sh), sizeof(sh)) != sizeof(sh))
    goto bad;
  
  // read in section names
  if (readi(ip, names, strtab.sh_offset, strtab.sh_size)<0)
    goto bad;
  
  // check section headers for address of the text section
  for (i=0, off=elf.shoff; i<elf.shnum; i++, off+= sizeof(sh)){
    if (readi(ip, (char*) &sh, off, sizeof(sh)) != sizeof(sh))
      goto bad; 
    firstindex = (i==0) ? sh.sh_name + 1 : sh.sh_name;
    if (strncmp(names + firstindex * sizeof(char), ".text", 5) == 0) {
      textaddr = sh.sh_addr;
      textsz = sh.sh_size;
    }
  }

  // text not found or at wrong address
  if (textaddr != 0x1000) 
    goto bad;

  // number of pages that are text only
  numtextpage = (textsz - (textsz % PGSIZE)) / PGSIZE;

  // this reads the whole program into memory
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    // read the current prog reader into ph
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    vaddr = ph.vaddr;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(vaddr + ph.memsz < vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, vaddr + ph.memsz)) == 0)
      goto bad;
    if(vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  sz = PGROUNDUP(sz);

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));



  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  curproc->numtextpage = numtextpage;
  
  // start the clock ticks fresh for user program
  curproc->ticks = 0;
  acquire(&tickslock);
  curproc->starttick = ticks;
  release(&tickslock);
  switchuvm(curproc);

  // make sure all changes seen by hw
  lcr3(V2P(curproc->pgdir));
  freevm(oldpgdir);
  
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
