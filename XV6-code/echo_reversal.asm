
_echo_reversal:     file format elf32-i386


Disassembly of section .text:

00000000 <main>:
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
   0:	8d 4c 24 04          	lea    0x4(%esp),%ecx
   4:	83 e4 f0             	and    $0xfffffff0,%esp
   7:	ff 71 fc             	pushl  -0x4(%ecx)
   a:	55                   	push   %ebp
   b:	89 e5                	mov    %esp,%ebp
   d:	57                   	push   %edi
   e:	56                   	push   %esi
   f:	53                   	push   %ebx
  10:	51                   	push   %ecx
  11:	83 ec 18             	sub    $0x18,%esp
  14:	8b 01                	mov    (%ecx),%eax
  16:	8b 71 04             	mov    0x4(%ecx),%esi
    for(int i = 1; i < argc; i++){
  19:	83 f8 01             	cmp    $0x1,%eax
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  1c:	89 45 e4             	mov    %eax,-0x1c(%ebp)
    for(int i = 1; i < argc; i++){
  1f:	0f 8e 83 00 00 00    	jle    a8 <main+0xa8>
  25:	bb 01 00 00 00       	mov    $0x1,%ebx
  2a:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
        int count=0;
        while(1){
            if(argv[i][count]=='\0')    //find out the end
  30:	8b 14 9e             	mov    (%esi,%ebx,4),%edx
  33:	80 3a 00             	cmpb   $0x0,(%edx)
  36:	74 3a                	je     72 <main+0x72>
  38:	31 ff                	xor    %edi,%edi
  3a:	eb 06                	jmp    42 <main+0x42>
  3c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
                break;
            else
                count++;
  40:	89 cf                	mov    %ecx,%edi
int main(int argc, char *argv[])
{
    for(int i = 1; i < argc; i++){
        int count=0;
        while(1){
            if(argv[i][count]=='\0')    //find out the end
  42:	80 7c 3a 01 00       	cmpb   $0x0,0x1(%edx,%edi,1)
                break;
            else
                count++;
  47:	8d 4f 01             	lea    0x1(%edi),%ecx
int main(int argc, char *argv[])
{
    for(int i = 1; i < argc; i++){
        int count=0;
        while(1){
            if(argv[i][count]=='\0')    //find out the end
  4a:	75 f4                	jne    40 <main+0x40>
  4c:	eb 05                	jmp    53 <main+0x53>
  4e:	66 90                	xchg   %ax,%ax
  50:	8b 14 9e             	mov    (%esi,%ebx,4),%edx
                break;
            else
                count++;
        }
        for(int j=count-1;j>=0;j--)
            printf(1,"%c",argv[i][j]);  //reversal
  53:	0f be 14 3a          	movsbl (%edx,%edi,1),%edx
  57:	83 ec 04             	sub    $0x4,%esp
            if(argv[i][count]=='\0')    //find out the end
                break;
            else
                count++;
        }
        for(int j=count-1;j>=0;j--)
  5a:	83 ef 01             	sub    $0x1,%edi
            printf(1,"%c",argv[i][j]);  //reversal
  5d:	52                   	push   %edx
  5e:	68 60 07 00 00       	push   $0x760
  63:	6a 01                	push   $0x1
  65:	e8 d6 03 00 00       	call   440 <printf>
            if(argv[i][count]=='\0')    //find out the end
                break;
            else
                count++;
        }
        for(int j=count-1;j>=0;j--)
  6a:	83 c4 10             	add    $0x10,%esp
  6d:	83 ff ff             	cmp    $0xffffffff,%edi
  70:	75 de                	jne    50 <main+0x50>
            printf(1,"%c",argv[i][j]);  //reversal
        printf(1, "%s", i+1 < argc ? " " : "\n");
  72:	83 c3 01             	add    $0x1,%ebx
  75:	39 5d e4             	cmp    %ebx,-0x1c(%ebp)
  78:	74 19                	je     93 <main+0x93>
  7a:	83 ec 04             	sub    $0x4,%esp
  7d:	68 68 07 00 00       	push   $0x768
  82:	68 65 07 00 00       	push   $0x765
  87:	6a 01                	push   $0x1
  89:	e8 b2 03 00 00       	call   440 <printf>
  8e:	83 c4 10             	add    $0x10,%esp
  91:	eb 9d                	jmp    30 <main+0x30>
  93:	50                   	push   %eax
  94:	68 63 07 00 00       	push   $0x763
  99:	68 65 07 00 00       	push   $0x765
  9e:	6a 01                	push   $0x1
  a0:	e8 9b 03 00 00       	call   440 <printf>
  a5:	83 c4 10             	add    $0x10,%esp
    }
    exit();
  a8:	e8 45 02 00 00       	call   2f2 <exit>
  ad:	66 90                	xchg   %ax,%ax
  af:	90                   	nop

000000b0 <strcpy>:
#include "user.h"
#include "x86.h"

char*
strcpy(char *s, const char *t)
{
  b0:	55                   	push   %ebp
  b1:	89 e5                	mov    %esp,%ebp
  b3:	53                   	push   %ebx
  b4:	8b 45 08             	mov    0x8(%ebp),%eax
  b7:	8b 4d 0c             	mov    0xc(%ebp),%ecx
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
  ba:	89 c2                	mov    %eax,%edx
  bc:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
  c0:	83 c1 01             	add    $0x1,%ecx
  c3:	0f b6 59 ff          	movzbl -0x1(%ecx),%ebx
  c7:	83 c2 01             	add    $0x1,%edx
  ca:	84 db                	test   %bl,%bl
  cc:	88 5a ff             	mov    %bl,-0x1(%edx)
  cf:	75 ef                	jne    c0 <strcpy+0x10>
    ;
  return os;
}
  d1:	5b                   	pop    %ebx
  d2:	5d                   	pop    %ebp
  d3:	c3                   	ret    
  d4:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
  da:	8d bf 00 00 00 00    	lea    0x0(%edi),%edi

000000e0 <strcmp>:

int
strcmp(const char *p, const char *q)
{
  e0:	55                   	push   %ebp
  e1:	89 e5                	mov    %esp,%ebp
  e3:	56                   	push   %esi
  e4:	53                   	push   %ebx
  e5:	8b 55 08             	mov    0x8(%ebp),%edx
  e8:	8b 4d 0c             	mov    0xc(%ebp),%ecx
  while(*p && *p == *q)
  eb:	0f b6 02             	movzbl (%edx),%eax
  ee:	0f b6 19             	movzbl (%ecx),%ebx
  f1:	84 c0                	test   %al,%al
  f3:	75 1e                	jne    113 <strcmp+0x33>
  f5:	eb 29                	jmp    120 <strcmp+0x40>
  f7:	89 f6                	mov    %esi,%esi
  f9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
    p++, q++;
 100:	83 c2 01             	add    $0x1,%edx
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
 103:	0f b6 02             	movzbl (%edx),%eax
    p++, q++;
 106:	8d 71 01             	lea    0x1(%ecx),%esi
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
 109:	0f b6 59 01          	movzbl 0x1(%ecx),%ebx
 10d:	84 c0                	test   %al,%al
 10f:	74 0f                	je     120 <strcmp+0x40>
 111:	89 f1                	mov    %esi,%ecx
 113:	38 d8                	cmp    %bl,%al
 115:	74 e9                	je     100 <strcmp+0x20>
    p++, q++;
  return (uchar)*p - (uchar)*q;
 117:	29 d8                	sub    %ebx,%eax
}
 119:	5b                   	pop    %ebx
 11a:	5e                   	pop    %esi
 11b:	5d                   	pop    %ebp
 11c:	c3                   	ret    
 11d:	8d 76 00             	lea    0x0(%esi),%esi
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
 120:	31 c0                	xor    %eax,%eax
    p++, q++;
  return (uchar)*p - (uchar)*q;
 122:	29 d8                	sub    %ebx,%eax
}
 124:	5b                   	pop    %ebx
 125:	5e                   	pop    %esi
 126:	5d                   	pop    %ebp
 127:	c3                   	ret    
 128:	90                   	nop
 129:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

00000130 <strlen>:

uint
strlen(const char *s)
{
 130:	55                   	push   %ebp
 131:	89 e5                	mov    %esp,%ebp
 133:	8b 4d 08             	mov    0x8(%ebp),%ecx
  int n;

  for(n = 0; s[n]; n++)
 136:	80 39 00             	cmpb   $0x0,(%ecx)
 139:	74 12                	je     14d <strlen+0x1d>
 13b:	31 d2                	xor    %edx,%edx
 13d:	8d 76 00             	lea    0x0(%esi),%esi
 140:	83 c2 01             	add    $0x1,%edx
 143:	80 3c 11 00          	cmpb   $0x0,(%ecx,%edx,1)
 147:	89 d0                	mov    %edx,%eax
 149:	75 f5                	jne    140 <strlen+0x10>
    ;
  return n;
}
 14b:	5d                   	pop    %ebp
 14c:	c3                   	ret    
uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
 14d:	31 c0                	xor    %eax,%eax
    ;
  return n;
}
 14f:	5d                   	pop    %ebp
 150:	c3                   	ret    
 151:	eb 0d                	jmp    160 <memset>
 153:	90                   	nop
 154:	90                   	nop
 155:	90                   	nop
 156:	90                   	nop
 157:	90                   	nop
 158:	90                   	nop
 159:	90                   	nop
 15a:	90                   	nop
 15b:	90                   	nop
 15c:	90                   	nop
 15d:	90                   	nop
 15e:	90                   	nop
 15f:	90                   	nop

00000160 <memset>:

void*
memset(void *dst, int c, uint n)
{
 160:	55                   	push   %ebp
 161:	89 e5                	mov    %esp,%ebp
 163:	57                   	push   %edi
 164:	8b 55 08             	mov    0x8(%ebp),%edx
}

static inline void
stosb(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosb" :
 167:	8b 4d 10             	mov    0x10(%ebp),%ecx
 16a:	8b 45 0c             	mov    0xc(%ebp),%eax
 16d:	89 d7                	mov    %edx,%edi
 16f:	fc                   	cld    
 170:	f3 aa                	rep stos %al,%es:(%edi)
  stosb(dst, c, n);
  return dst;
}
 172:	89 d0                	mov    %edx,%eax
 174:	5f                   	pop    %edi
 175:	5d                   	pop    %ebp
 176:	c3                   	ret    
 177:	89 f6                	mov    %esi,%esi
 179:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000180 <strchr>:

char*
strchr(const char *s, char c)
{
 180:	55                   	push   %ebp
 181:	89 e5                	mov    %esp,%ebp
 183:	53                   	push   %ebx
 184:	8b 45 08             	mov    0x8(%ebp),%eax
 187:	8b 5d 0c             	mov    0xc(%ebp),%ebx
  for(; *s; s++)
 18a:	0f b6 10             	movzbl (%eax),%edx
 18d:	84 d2                	test   %dl,%dl
 18f:	74 1d                	je     1ae <strchr+0x2e>
    if(*s == c)
 191:	38 d3                	cmp    %dl,%bl
 193:	89 d9                	mov    %ebx,%ecx
 195:	75 0d                	jne    1a4 <strchr+0x24>
 197:	eb 17                	jmp    1b0 <strchr+0x30>
 199:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
 1a0:	38 ca                	cmp    %cl,%dl
 1a2:	74 0c                	je     1b0 <strchr+0x30>
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
 1a4:	83 c0 01             	add    $0x1,%eax
 1a7:	0f b6 10             	movzbl (%eax),%edx
 1aa:	84 d2                	test   %dl,%dl
 1ac:	75 f2                	jne    1a0 <strchr+0x20>
    if(*s == c)
      return (char*)s;
  return 0;
 1ae:	31 c0                	xor    %eax,%eax
}
 1b0:	5b                   	pop    %ebx
 1b1:	5d                   	pop    %ebp
 1b2:	c3                   	ret    
 1b3:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
 1b9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

000001c0 <gets>:

char*
gets(char *buf, int max)
{
 1c0:	55                   	push   %ebp
 1c1:	89 e5                	mov    %esp,%ebp
 1c3:	57                   	push   %edi
 1c4:	56                   	push   %esi
 1c5:	53                   	push   %ebx
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
 1c6:	31 f6                	xor    %esi,%esi
    cc = read(0, &c, 1);
 1c8:	8d 7d e7             	lea    -0x19(%ebp),%edi
  return 0;
}

char*
gets(char *buf, int max)
{
 1cb:	83 ec 1c             	sub    $0x1c,%esp
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
 1ce:	eb 29                	jmp    1f9 <gets+0x39>
    cc = read(0, &c, 1);
 1d0:	83 ec 04             	sub    $0x4,%esp
 1d3:	6a 01                	push   $0x1
 1d5:	57                   	push   %edi
 1d6:	6a 00                	push   $0x0
 1d8:	e8 2d 01 00 00       	call   30a <read>
    if(cc < 1)
 1dd:	83 c4 10             	add    $0x10,%esp
 1e0:	85 c0                	test   %eax,%eax
 1e2:	7e 1d                	jle    201 <gets+0x41>
      break;
    buf[i++] = c;
 1e4:	0f b6 45 e7          	movzbl -0x19(%ebp),%eax
 1e8:	8b 55 08             	mov    0x8(%ebp),%edx
 1eb:	89 de                	mov    %ebx,%esi
    if(c == '\n' || c == '\r')
 1ed:	3c 0a                	cmp    $0xa,%al

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
 1ef:	88 44 1a ff          	mov    %al,-0x1(%edx,%ebx,1)
    if(c == '\n' || c == '\r')
 1f3:	74 1b                	je     210 <gets+0x50>
 1f5:	3c 0d                	cmp    $0xd,%al
 1f7:	74 17                	je     210 <gets+0x50>
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
 1f9:	8d 5e 01             	lea    0x1(%esi),%ebx
 1fc:	3b 5d 0c             	cmp    0xc(%ebp),%ebx
 1ff:	7c cf                	jl     1d0 <gets+0x10>
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
 201:	8b 45 08             	mov    0x8(%ebp),%eax
 204:	c6 04 30 00          	movb   $0x0,(%eax,%esi,1)
  return buf;
}
 208:	8d 65 f4             	lea    -0xc(%ebp),%esp
 20b:	5b                   	pop    %ebx
 20c:	5e                   	pop    %esi
 20d:	5f                   	pop    %edi
 20e:	5d                   	pop    %ebp
 20f:	c3                   	ret    
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
 210:	8b 45 08             	mov    0x8(%ebp),%eax
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
 213:	89 de                	mov    %ebx,%esi
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
 215:	c6 04 30 00          	movb   $0x0,(%eax,%esi,1)
  return buf;
}
 219:	8d 65 f4             	lea    -0xc(%ebp),%esp
 21c:	5b                   	pop    %ebx
 21d:	5e                   	pop    %esi
 21e:	5f                   	pop    %edi
 21f:	5d                   	pop    %ebp
 220:	c3                   	ret    
 221:	eb 0d                	jmp    230 <stat>
 223:	90                   	nop
 224:	90                   	nop
 225:	90                   	nop
 226:	90                   	nop
 227:	90                   	nop
 228:	90                   	nop
 229:	90                   	nop
 22a:	90                   	nop
 22b:	90                   	nop
 22c:	90                   	nop
 22d:	90                   	nop
 22e:	90                   	nop
 22f:	90                   	nop

00000230 <stat>:

int
stat(const char *n, struct stat *st)
{
 230:	55                   	push   %ebp
 231:	89 e5                	mov    %esp,%ebp
 233:	56                   	push   %esi
 234:	53                   	push   %ebx
  int fd;
  int r;

  fd = open(n, O_RDONLY);
 235:	83 ec 08             	sub    $0x8,%esp
 238:	6a 00                	push   $0x0
 23a:	ff 75 08             	pushl  0x8(%ebp)
 23d:	e8 f0 00 00 00       	call   332 <open>
  if(fd < 0)
 242:	83 c4 10             	add    $0x10,%esp
 245:	85 c0                	test   %eax,%eax
 247:	78 27                	js     270 <stat+0x40>
    return -1;
  r = fstat(fd, st);
 249:	83 ec 08             	sub    $0x8,%esp
 24c:	ff 75 0c             	pushl  0xc(%ebp)
 24f:	89 c3                	mov    %eax,%ebx
 251:	50                   	push   %eax
 252:	e8 f3 00 00 00       	call   34a <fstat>
 257:	89 c6                	mov    %eax,%esi
  close(fd);
 259:	89 1c 24             	mov    %ebx,(%esp)
 25c:	e8 b9 00 00 00       	call   31a <close>
  return r;
 261:	83 c4 10             	add    $0x10,%esp
 264:	89 f0                	mov    %esi,%eax
}
 266:	8d 65 f8             	lea    -0x8(%ebp),%esp
 269:	5b                   	pop    %ebx
 26a:	5e                   	pop    %esi
 26b:	5d                   	pop    %ebp
 26c:	c3                   	ret    
 26d:	8d 76 00             	lea    0x0(%esi),%esi
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
 270:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
 275:	eb ef                	jmp    266 <stat+0x36>
 277:	89 f6                	mov    %esi,%esi
 279:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi

00000280 <atoi>:
  return r;
}

int
atoi(const char *s)
{
 280:	55                   	push   %ebp
 281:	89 e5                	mov    %esp,%ebp
 283:	53                   	push   %ebx
 284:	8b 4d 08             	mov    0x8(%ebp),%ecx
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
 287:	0f be 11             	movsbl (%ecx),%edx
 28a:	8d 42 d0             	lea    -0x30(%edx),%eax
 28d:	3c 09                	cmp    $0x9,%al
 28f:	b8 00 00 00 00       	mov    $0x0,%eax
 294:	77 1f                	ja     2b5 <atoi+0x35>
 296:	8d 76 00             	lea    0x0(%esi),%esi
 299:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
    n = n*10 + *s++ - '0';
 2a0:	8d 04 80             	lea    (%eax,%eax,4),%eax
 2a3:	83 c1 01             	add    $0x1,%ecx
 2a6:	8d 44 42 d0          	lea    -0x30(%edx,%eax,2),%eax
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
 2aa:	0f be 11             	movsbl (%ecx),%edx
 2ad:	8d 5a d0             	lea    -0x30(%edx),%ebx
 2b0:	80 fb 09             	cmp    $0x9,%bl
 2b3:	76 eb                	jbe    2a0 <atoi+0x20>
    n = n*10 + *s++ - '0';
  return n;
}
 2b5:	5b                   	pop    %ebx
 2b6:	5d                   	pop    %ebp
 2b7:	c3                   	ret    
 2b8:	90                   	nop
 2b9:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi

000002c0 <memmove>:

void*
memmove(void *vdst, const void *vsrc, int n)
{
 2c0:	55                   	push   %ebp
 2c1:	89 e5                	mov    %esp,%ebp
 2c3:	56                   	push   %esi
 2c4:	53                   	push   %ebx
 2c5:	8b 5d 10             	mov    0x10(%ebp),%ebx
 2c8:	8b 45 08             	mov    0x8(%ebp),%eax
 2cb:	8b 75 0c             	mov    0xc(%ebp),%esi
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
 2ce:	85 db                	test   %ebx,%ebx
 2d0:	7e 14                	jle    2e6 <memmove+0x26>
 2d2:	31 d2                	xor    %edx,%edx
 2d4:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    *dst++ = *src++;
 2d8:	0f b6 0c 16          	movzbl (%esi,%edx,1),%ecx
 2dc:	88 0c 10             	mov    %cl,(%eax,%edx,1)
 2df:	83 c2 01             	add    $0x1,%edx
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
 2e2:	39 da                	cmp    %ebx,%edx
 2e4:	75 f2                	jne    2d8 <memmove+0x18>
    *dst++ = *src++;
  return vdst;
}
 2e6:	5b                   	pop    %ebx
 2e7:	5e                   	pop    %esi
 2e8:	5d                   	pop    %ebp
 2e9:	c3                   	ret    

000002ea <fork>:
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
 2ea:	b8 01 00 00 00       	mov    $0x1,%eax
 2ef:	cd 40                	int    $0x40
 2f1:	c3                   	ret    

000002f2 <exit>:
SYSCALL(exit)
 2f2:	b8 02 00 00 00       	mov    $0x2,%eax
 2f7:	cd 40                	int    $0x40
 2f9:	c3                   	ret    

000002fa <wait>:
SYSCALL(wait)
 2fa:	b8 03 00 00 00       	mov    $0x3,%eax
 2ff:	cd 40                	int    $0x40
 301:	c3                   	ret    

00000302 <pipe>:
SYSCALL(pipe)
 302:	b8 04 00 00 00       	mov    $0x4,%eax
 307:	cd 40                	int    $0x40
 309:	c3                   	ret    

0000030a <read>:
SYSCALL(read)
 30a:	b8 05 00 00 00       	mov    $0x5,%eax
 30f:	cd 40                	int    $0x40
 311:	c3                   	ret    

00000312 <write>:
SYSCALL(write)
 312:	b8 10 00 00 00       	mov    $0x10,%eax
 317:	cd 40                	int    $0x40
 319:	c3                   	ret    

0000031a <close>:
SYSCALL(close)
 31a:	b8 15 00 00 00       	mov    $0x15,%eax
 31f:	cd 40                	int    $0x40
 321:	c3                   	ret    

00000322 <kill>:
SYSCALL(kill)
 322:	b8 06 00 00 00       	mov    $0x6,%eax
 327:	cd 40                	int    $0x40
 329:	c3                   	ret    

0000032a <exec>:
SYSCALL(exec)
 32a:	b8 07 00 00 00       	mov    $0x7,%eax
 32f:	cd 40                	int    $0x40
 331:	c3                   	ret    

00000332 <open>:
SYSCALL(open)
 332:	b8 0f 00 00 00       	mov    $0xf,%eax
 337:	cd 40                	int    $0x40
 339:	c3                   	ret    

0000033a <mknod>:
SYSCALL(mknod)
 33a:	b8 11 00 00 00       	mov    $0x11,%eax
 33f:	cd 40                	int    $0x40
 341:	c3                   	ret    

00000342 <unlink>:
SYSCALL(unlink)
 342:	b8 12 00 00 00       	mov    $0x12,%eax
 347:	cd 40                	int    $0x40
 349:	c3                   	ret    

0000034a <fstat>:
SYSCALL(fstat)
 34a:	b8 08 00 00 00       	mov    $0x8,%eax
 34f:	cd 40                	int    $0x40
 351:	c3                   	ret    

00000352 <link>:
SYSCALL(link)
 352:	b8 13 00 00 00       	mov    $0x13,%eax
 357:	cd 40                	int    $0x40
 359:	c3                   	ret    

0000035a <mkdir>:
SYSCALL(mkdir)
 35a:	b8 14 00 00 00       	mov    $0x14,%eax
 35f:	cd 40                	int    $0x40
 361:	c3                   	ret    

00000362 <chdir>:
SYSCALL(chdir)
 362:	b8 09 00 00 00       	mov    $0x9,%eax
 367:	cd 40                	int    $0x40
 369:	c3                   	ret    

0000036a <dup>:
SYSCALL(dup)
 36a:	b8 0a 00 00 00       	mov    $0xa,%eax
 36f:	cd 40                	int    $0x40
 371:	c3                   	ret    

00000372 <getpid>:
SYSCALL(getpid)
 372:	b8 0b 00 00 00       	mov    $0xb,%eax
 377:	cd 40                	int    $0x40
 379:	c3                   	ret    

0000037a <sbrk>:
SYSCALL(sbrk)
 37a:	b8 0c 00 00 00       	mov    $0xc,%eax
 37f:	cd 40                	int    $0x40
 381:	c3                   	ret    

00000382 <sleep>:
SYSCALL(sleep)
 382:	b8 0d 00 00 00       	mov    $0xd,%eax
 387:	cd 40                	int    $0x40
 389:	c3                   	ret    

0000038a <uptime>:
SYSCALL(uptime)
 38a:	b8 0e 00 00 00       	mov    $0xe,%eax
 38f:	cd 40                	int    $0x40
 391:	c3                   	ret    
 392:	66 90                	xchg   %ax,%ax
 394:	66 90                	xchg   %ax,%ax
 396:	66 90                	xchg   %ax,%ax
 398:	66 90                	xchg   %ax,%ax
 39a:	66 90                	xchg   %ax,%ax
 39c:	66 90                	xchg   %ax,%ax
 39e:	66 90                	xchg   %ax,%ax

000003a0 <printint>:
    write(fd, &c, 1);
}

    static void
printint(int fd, int xx, int base, int sgn)
{
 3a0:	55                   	push   %ebp
 3a1:	89 e5                	mov    %esp,%ebp
 3a3:	57                   	push   %edi
 3a4:	56                   	push   %esi
 3a5:	53                   	push   %ebx
 3a6:	89 c6                	mov    %eax,%esi
 3a8:	83 ec 3c             	sub    $0x3c,%esp
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if(sgn && xx < 0){
 3ab:	8b 5d 08             	mov    0x8(%ebp),%ebx
 3ae:	85 db                	test   %ebx,%ebx
 3b0:	74 7e                	je     430 <printint+0x90>
 3b2:	89 d0                	mov    %edx,%eax
 3b4:	c1 e8 1f             	shr    $0x1f,%eax
 3b7:	84 c0                	test   %al,%al
 3b9:	74 75                	je     430 <printint+0x90>
        neg = 1;
        x = -xx;
 3bb:	89 d0                	mov    %edx,%eax
    int i, neg;
    uint x;

    neg = 0;
    if(sgn && xx < 0){
        neg = 1;
 3bd:	c7 45 c4 01 00 00 00 	movl   $0x1,-0x3c(%ebp)
        x = -xx;
 3c4:	f7 d8                	neg    %eax
 3c6:	89 75 c0             	mov    %esi,-0x40(%ebp)
    } else {
        x = xx;
    }

    i = 0;
 3c9:	31 ff                	xor    %edi,%edi
 3cb:	8d 5d d7             	lea    -0x29(%ebp),%ebx
 3ce:	89 ce                	mov    %ecx,%esi
 3d0:	eb 08                	jmp    3da <printint+0x3a>
 3d2:	8d b6 00 00 00 00    	lea    0x0(%esi),%esi
    do{
        buf[i++] = digits[x % base];
 3d8:	89 cf                	mov    %ecx,%edi
 3da:	31 d2                	xor    %edx,%edx
 3dc:	8d 4f 01             	lea    0x1(%edi),%ecx
 3df:	f7 f6                	div    %esi
 3e1:	0f b6 92 74 07 00 00 	movzbl 0x774(%edx),%edx
    }while((x /= base) != 0);
 3e8:	85 c0                	test   %eax,%eax
        x = xx;
    }

    i = 0;
    do{
        buf[i++] = digits[x % base];
 3ea:	88 14 0b             	mov    %dl,(%ebx,%ecx,1)
    }while((x /= base) != 0);
 3ed:	75 e9                	jne    3d8 <printint+0x38>
    if(neg)
 3ef:	8b 45 c4             	mov    -0x3c(%ebp),%eax
 3f2:	8b 75 c0             	mov    -0x40(%ebp),%esi
 3f5:	85 c0                	test   %eax,%eax
 3f7:	74 08                	je     401 <printint+0x61>
        buf[i++] = '-';
 3f9:	c6 44 0d d8 2d       	movb   $0x2d,-0x28(%ebp,%ecx,1)
 3fe:	8d 4f 02             	lea    0x2(%edi),%ecx
 401:	8d 7c 0d d7          	lea    -0x29(%ebp,%ecx,1),%edi
 405:	8d 76 00             	lea    0x0(%esi),%esi
 408:	0f b6 07             	movzbl (%edi),%eax
#include "user.h"

    static void
putc(int fd, char c)
{
    write(fd, &c, 1);
 40b:	83 ec 04             	sub    $0x4,%esp
 40e:	83 ef 01             	sub    $0x1,%edi
 411:	6a 01                	push   $0x1
 413:	53                   	push   %ebx
 414:	56                   	push   %esi
 415:	88 45 d7             	mov    %al,-0x29(%ebp)
 418:	e8 f5 fe ff ff       	call   312 <write>
        buf[i++] = digits[x % base];
    }while((x /= base) != 0);
    if(neg)
        buf[i++] = '-';

    while(--i >= 0)
 41d:	83 c4 10             	add    $0x10,%esp
 420:	39 df                	cmp    %ebx,%edi
 422:	75 e4                	jne    408 <printint+0x68>
        putc(fd, buf[i]);
}
 424:	8d 65 f4             	lea    -0xc(%ebp),%esp
 427:	5b                   	pop    %ebx
 428:	5e                   	pop    %esi
 429:	5f                   	pop    %edi
 42a:	5d                   	pop    %ebp
 42b:	c3                   	ret    
 42c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    neg = 0;
    if(sgn && xx < 0){
        neg = 1;
        x = -xx;
    } else {
        x = xx;
 430:	89 d0                	mov    %edx,%eax
    static char digits[] = "0123456789ABCDEF";
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
 432:	c7 45 c4 00 00 00 00 	movl   $0x0,-0x3c(%ebp)
 439:	eb 8b                	jmp    3c6 <printint+0x26>
 43b:	90                   	nop
 43c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi

00000440 <printf>:
}

// Print to the given fd. Only understands %d, %x, %p, %s.
    void
printf(int fd, const char *fmt, ...)
{
 440:	55                   	push   %ebp
 441:	89 e5                	mov    %esp,%ebp
 443:	57                   	push   %edi
 444:	56                   	push   %esi
 445:	53                   	push   %ebx
    int c, i, state;
    uint *ap;

    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
 446:	8d 45 10             	lea    0x10(%ebp),%eax
}

// Print to the given fd. Only understands %d, %x, %p, %s.
    void
printf(int fd, const char *fmt, ...)
{
 449:	83 ec 2c             	sub    $0x2c,%esp
    int c, i, state;
    uint *ap;

    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
 44c:	8b 75 0c             	mov    0xc(%ebp),%esi
}

// Print to the given fd. Only understands %d, %x, %p, %s.
    void
printf(int fd, const char *fmt, ...)
{
 44f:	8b 7d 08             	mov    0x8(%ebp),%edi
    int c, i, state;
    uint *ap;

    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
 452:	89 45 d0             	mov    %eax,-0x30(%ebp)
 455:	0f b6 1e             	movzbl (%esi),%ebx
 458:	83 c6 01             	add    $0x1,%esi
 45b:	84 db                	test   %bl,%bl
 45d:	0f 84 b0 00 00 00    	je     513 <printf+0xd3>
 463:	31 d2                	xor    %edx,%edx
 465:	eb 39                	jmp    4a0 <printf+0x60>
 467:	89 f6                	mov    %esi,%esi
 469:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
        c = fmt[i] & 0xff;
        if(state == 0){
            if(c == '%'){
 470:	83 f8 25             	cmp    $0x25,%eax
 473:	89 55 d4             	mov    %edx,-0x2c(%ebp)
                state = '%';
 476:	ba 25 00 00 00       	mov    $0x25,%edx
    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
        c = fmt[i] & 0xff;
        if(state == 0){
            if(c == '%'){
 47b:	74 18                	je     495 <printf+0x55>
#include "user.h"

    static void
putc(int fd, char c)
{
    write(fd, &c, 1);
 47d:	8d 45 e2             	lea    -0x1e(%ebp),%eax
 480:	83 ec 04             	sub    $0x4,%esp
 483:	88 5d e2             	mov    %bl,-0x1e(%ebp)
 486:	6a 01                	push   $0x1
 488:	50                   	push   %eax
 489:	57                   	push   %edi
 48a:	e8 83 fe ff ff       	call   312 <write>
 48f:	8b 55 d4             	mov    -0x2c(%ebp),%edx
 492:	83 c4 10             	add    $0x10,%esp
 495:	83 c6 01             	add    $0x1,%esi
    int c, i, state;
    uint *ap;

    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
 498:	0f b6 5e ff          	movzbl -0x1(%esi),%ebx
 49c:	84 db                	test   %bl,%bl
 49e:	74 73                	je     513 <printf+0xd3>
        c = fmt[i] & 0xff;
        if(state == 0){
 4a0:	85 d2                	test   %edx,%edx
    uint *ap;

    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
        c = fmt[i] & 0xff;
 4a2:	0f be cb             	movsbl %bl,%ecx
 4a5:	0f b6 c3             	movzbl %bl,%eax
        if(state == 0){
 4a8:	74 c6                	je     470 <printf+0x30>
            if(c == '%'){
                state = '%';
            } else {
                putc(fd, c);
            }
        } else if(state == '%'){
 4aa:	83 fa 25             	cmp    $0x25,%edx
 4ad:	75 e6                	jne    495 <printf+0x55>
            if(c == 'd'){
 4af:	83 f8 64             	cmp    $0x64,%eax
 4b2:	0f 84 f8 00 00 00    	je     5b0 <printf+0x170>
                printint(fd, *ap, 10, 1);
                ap++;
            } else if(c == 'x' || c == 'p'){
 4b8:	81 e1 f7 00 00 00    	and    $0xf7,%ecx
 4be:	83 f9 70             	cmp    $0x70,%ecx
 4c1:	74 5d                	je     520 <printf+0xe0>
                printint(fd, *ap, 16, 0);
                ap++;
            } else if(c == 's'){
 4c3:	83 f8 73             	cmp    $0x73,%eax
 4c6:	0f 84 84 00 00 00    	je     550 <printf+0x110>
                    s = "(null)";
                while(*s != 0){
                    putc(fd, *s);
                    s++;
                }
            } else if(c == 'c'){
 4cc:	83 f8 63             	cmp    $0x63,%eax
 4cf:	0f 84 ea 00 00 00    	je     5bf <printf+0x17f>
                putc(fd, *ap);
                ap++;
            } else if(c == '%'){
 4d5:	83 f8 25             	cmp    $0x25,%eax
 4d8:	0f 84 c2 00 00 00    	je     5a0 <printf+0x160>
#include "user.h"

    static void
putc(int fd, char c)
{
    write(fd, &c, 1);
 4de:	8d 45 e7             	lea    -0x19(%ebp),%eax
 4e1:	83 ec 04             	sub    $0x4,%esp
 4e4:	c6 45 e7 25          	movb   $0x25,-0x19(%ebp)
 4e8:	6a 01                	push   $0x1
 4ea:	50                   	push   %eax
 4eb:	57                   	push   %edi
 4ec:	e8 21 fe ff ff       	call   312 <write>
 4f1:	83 c4 0c             	add    $0xc,%esp
 4f4:	8d 45 e6             	lea    -0x1a(%ebp),%eax
 4f7:	88 5d e6             	mov    %bl,-0x1a(%ebp)
 4fa:	6a 01                	push   $0x1
 4fc:	50                   	push   %eax
 4fd:	57                   	push   %edi
 4fe:	83 c6 01             	add    $0x1,%esi
 501:	e8 0c fe ff ff       	call   312 <write>
    int c, i, state;
    uint *ap;

    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
 506:	0f b6 5e ff          	movzbl -0x1(%esi),%ebx
#include "user.h"

    static void
putc(int fd, char c)
{
    write(fd, &c, 1);
 50a:	83 c4 10             	add    $0x10,%esp
            } else {
                // Unknown % sequence.  Print it to draw attention.
                putc(fd, '%');
                putc(fd, c);
            }
            state = 0;
 50d:	31 d2                	xor    %edx,%edx
    int c, i, state;
    uint *ap;

    state = 0;
    ap = (uint*)(void*)&fmt + 1;
    for(i = 0; fmt[i]; i++){
 50f:	84 db                	test   %bl,%bl
 511:	75 8d                	jne    4a0 <printf+0x60>
                putc(fd, c);
            }
            state = 0;
        }
    }
}
 513:	8d 65 f4             	lea    -0xc(%ebp),%esp
 516:	5b                   	pop    %ebx
 517:	5e                   	pop    %esi
 518:	5f                   	pop    %edi
 519:	5d                   	pop    %ebp
 51a:	c3                   	ret    
 51b:	90                   	nop
 51c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
        } else if(state == '%'){
            if(c == 'd'){
                printint(fd, *ap, 10, 1);
                ap++;
            } else if(c == 'x' || c == 'p'){
                printint(fd, *ap, 16, 0);
 520:	83 ec 0c             	sub    $0xc,%esp
 523:	b9 10 00 00 00       	mov    $0x10,%ecx
 528:	6a 00                	push   $0x0
 52a:	8b 5d d0             	mov    -0x30(%ebp),%ebx
 52d:	89 f8                	mov    %edi,%eax
 52f:	8b 13                	mov    (%ebx),%edx
 531:	e8 6a fe ff ff       	call   3a0 <printint>
                ap++;
 536:	89 d8                	mov    %ebx,%eax
 538:	83 c4 10             	add    $0x10,%esp
            } else {
                // Unknown % sequence.  Print it to draw attention.
                putc(fd, '%');
                putc(fd, c);
            }
            state = 0;
 53b:	31 d2                	xor    %edx,%edx
            if(c == 'd'){
                printint(fd, *ap, 10, 1);
                ap++;
            } else if(c == 'x' || c == 'p'){
                printint(fd, *ap, 16, 0);
                ap++;
 53d:	83 c0 04             	add    $0x4,%eax
 540:	89 45 d0             	mov    %eax,-0x30(%ebp)
 543:	e9 4d ff ff ff       	jmp    495 <printf+0x55>
 548:	90                   	nop
 549:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
            } else if(c == 's'){
                s = (char*)*ap;
 550:	8b 45 d0             	mov    -0x30(%ebp),%eax
 553:	8b 18                	mov    (%eax),%ebx
                ap++;
 555:	83 c0 04             	add    $0x4,%eax
 558:	89 45 d0             	mov    %eax,-0x30(%ebp)
                if(s == 0)
                    s = "(null)";
 55b:	b8 6a 07 00 00       	mov    $0x76a,%eax
 560:	85 db                	test   %ebx,%ebx
 562:	0f 44 d8             	cmove  %eax,%ebx
                while(*s != 0){
 565:	0f b6 03             	movzbl (%ebx),%eax
 568:	84 c0                	test   %al,%al
 56a:	74 23                	je     58f <printf+0x14f>
 56c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
 570:	88 45 e3             	mov    %al,-0x1d(%ebp)
#include "user.h"

    static void
putc(int fd, char c)
{
    write(fd, &c, 1);
 573:	8d 45 e3             	lea    -0x1d(%ebp),%eax
 576:	83 ec 04             	sub    $0x4,%esp
 579:	6a 01                	push   $0x1
                ap++;
                if(s == 0)
                    s = "(null)";
                while(*s != 0){
                    putc(fd, *s);
                    s++;
 57b:	83 c3 01             	add    $0x1,%ebx
#include "user.h"

    static void
putc(int fd, char c)
{
    write(fd, &c, 1);
 57e:	50                   	push   %eax
 57f:	57                   	push   %edi
 580:	e8 8d fd ff ff       	call   312 <write>
            } else if(c == 's'){
                s = (char*)*ap;
                ap++;
                if(s == 0)
                    s = "(null)";
                while(*s != 0){
 585:	0f b6 03             	movzbl (%ebx),%eax
 588:	83 c4 10             	add    $0x10,%esp
 58b:	84 c0                	test   %al,%al
 58d:	75 e1                	jne    570 <printf+0x130>
            } else {
                // Unknown % sequence.  Print it to draw attention.
                putc(fd, '%');
                putc(fd, c);
            }
            state = 0;
 58f:	31 d2                	xor    %edx,%edx
 591:	e9 ff fe ff ff       	jmp    495 <printf+0x55>
 596:	8d 76 00             	lea    0x0(%esi),%esi
 599:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
#include "user.h"

    static void
putc(int fd, char c)
{
    write(fd, &c, 1);
 5a0:	83 ec 04             	sub    $0x4,%esp
 5a3:	88 5d e5             	mov    %bl,-0x1b(%ebp)
 5a6:	8d 45 e5             	lea    -0x1b(%ebp),%eax
 5a9:	6a 01                	push   $0x1
 5ab:	e9 4c ff ff ff       	jmp    4fc <printf+0xbc>
            } else {
                putc(fd, c);
            }
        } else if(state == '%'){
            if(c == 'd'){
                printint(fd, *ap, 10, 1);
 5b0:	83 ec 0c             	sub    $0xc,%esp
 5b3:	b9 0a 00 00 00       	mov    $0xa,%ecx
 5b8:	6a 01                	push   $0x1
 5ba:	e9 6b ff ff ff       	jmp    52a <printf+0xea>
 5bf:	8b 5d d0             	mov    -0x30(%ebp),%ebx
#include "user.h"

    static void
putc(int fd, char c)
{
    write(fd, &c, 1);
 5c2:	83 ec 04             	sub    $0x4,%esp
 5c5:	8b 03                	mov    (%ebx),%eax
 5c7:	6a 01                	push   $0x1
 5c9:	88 45 e4             	mov    %al,-0x1c(%ebp)
 5cc:	8d 45 e4             	lea    -0x1c(%ebp),%eax
 5cf:	50                   	push   %eax
 5d0:	57                   	push   %edi
 5d1:	e8 3c fd ff ff       	call   312 <write>
 5d6:	e9 5b ff ff ff       	jmp    536 <printf+0xf6>
 5db:	66 90                	xchg   %ax,%ax
 5dd:	66 90                	xchg   %ax,%ax
 5df:	90                   	nop

000005e0 <free>:
static Header base;
static Header *freep;

void
free(void *ap)
{
 5e0:	55                   	push   %ebp
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 5e1:	a1 18 0a 00 00       	mov    0xa18,%eax
static Header base;
static Header *freep;

void
free(void *ap)
{
 5e6:	89 e5                	mov    %esp,%ebp
 5e8:	57                   	push   %edi
 5e9:	56                   	push   %esi
 5ea:	53                   	push   %ebx
 5eb:	8b 5d 08             	mov    0x8(%ebp),%ebx
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 5ee:	8b 10                	mov    (%eax),%edx
void
free(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
 5f0:	8d 4b f8             	lea    -0x8(%ebx),%ecx
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 5f3:	39 c8                	cmp    %ecx,%eax
 5f5:	73 19                	jae    610 <free+0x30>
 5f7:	89 f6                	mov    %esi,%esi
 5f9:	8d bc 27 00 00 00 00 	lea    0x0(%edi,%eiz,1),%edi
 600:	39 d1                	cmp    %edx,%ecx
 602:	72 1c                	jb     620 <free+0x40>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 604:	39 d0                	cmp    %edx,%eax
 606:	73 18                	jae    620 <free+0x40>
static Header base;
static Header *freep;

void
free(void *ap)
{
 608:	89 d0                	mov    %edx,%eax
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 60a:	39 c8                	cmp    %ecx,%eax
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 60c:	8b 10                	mov    (%eax),%edx
free(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
 60e:	72 f0                	jb     600 <free+0x20>
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
 610:	39 d0                	cmp    %edx,%eax
 612:	72 f4                	jb     608 <free+0x28>
 614:	39 d1                	cmp    %edx,%ecx
 616:	73 f0                	jae    608 <free+0x28>
 618:	90                   	nop
 619:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
      break;
  if(bp + bp->s.size == p->s.ptr){
 620:	8b 73 fc             	mov    -0x4(%ebx),%esi
 623:	8d 3c f1             	lea    (%ecx,%esi,8),%edi
 626:	39 d7                	cmp    %edx,%edi
 628:	74 19                	je     643 <free+0x63>
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
 62a:	89 53 f8             	mov    %edx,-0x8(%ebx)
  if(p + p->s.size == bp){
 62d:	8b 50 04             	mov    0x4(%eax),%edx
 630:	8d 34 d0             	lea    (%eax,%edx,8),%esi
 633:	39 f1                	cmp    %esi,%ecx
 635:	74 23                	je     65a <free+0x7a>
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
 637:	89 08                	mov    %ecx,(%eax)
  freep = p;
 639:	a3 18 0a 00 00       	mov    %eax,0xa18
}
 63e:	5b                   	pop    %ebx
 63f:	5e                   	pop    %esi
 640:	5f                   	pop    %edi
 641:	5d                   	pop    %ebp
 642:	c3                   	ret    
  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
 643:	03 72 04             	add    0x4(%edx),%esi
 646:	89 73 fc             	mov    %esi,-0x4(%ebx)
    bp->s.ptr = p->s.ptr->s.ptr;
 649:	8b 10                	mov    (%eax),%edx
 64b:	8b 12                	mov    (%edx),%edx
 64d:	89 53 f8             	mov    %edx,-0x8(%ebx)
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
 650:	8b 50 04             	mov    0x4(%eax),%edx
 653:	8d 34 d0             	lea    (%eax,%edx,8),%esi
 656:	39 f1                	cmp    %esi,%ecx
 658:	75 dd                	jne    637 <free+0x57>
    p->s.size += bp->s.size;
 65a:	03 53 fc             	add    -0x4(%ebx),%edx
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
 65d:	a3 18 0a 00 00       	mov    %eax,0xa18
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
 662:	89 50 04             	mov    %edx,0x4(%eax)
    p->s.ptr = bp->s.ptr;
 665:	8b 53 f8             	mov    -0x8(%ebx),%edx
 668:	89 10                	mov    %edx,(%eax)
  } else
    p->s.ptr = bp;
  freep = p;
}
 66a:	5b                   	pop    %ebx
 66b:	5e                   	pop    %esi
 66c:	5f                   	pop    %edi
 66d:	5d                   	pop    %ebp
 66e:	c3                   	ret    
 66f:	90                   	nop

00000670 <malloc>:
  return freep;
}

void*
malloc(uint nbytes)
{
 670:	55                   	push   %ebp
 671:	89 e5                	mov    %esp,%ebp
 673:	57                   	push   %edi
 674:	56                   	push   %esi
 675:	53                   	push   %ebx
 676:	83 ec 0c             	sub    $0xc,%esp
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
 679:	8b 45 08             	mov    0x8(%ebp),%eax
  if((prevp = freep) == 0){
 67c:	8b 15 18 0a 00 00    	mov    0xa18,%edx
malloc(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
 682:	8d 78 07             	lea    0x7(%eax),%edi
 685:	c1 ef 03             	shr    $0x3,%edi
 688:	83 c7 01             	add    $0x1,%edi
  if((prevp = freep) == 0){
 68b:	85 d2                	test   %edx,%edx
 68d:	0f 84 a3 00 00 00    	je     736 <malloc+0xc6>
 693:	8b 02                	mov    (%edx),%eax
 695:	8b 48 04             	mov    0x4(%eax),%ecx
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
 698:	39 cf                	cmp    %ecx,%edi
 69a:	76 74                	jbe    710 <malloc+0xa0>
 69c:	81 ff 00 10 00 00    	cmp    $0x1000,%edi
 6a2:	be 00 10 00 00       	mov    $0x1000,%esi
 6a7:	8d 1c fd 00 00 00 00 	lea    0x0(,%edi,8),%ebx
 6ae:	0f 43 f7             	cmovae %edi,%esi
 6b1:	ba 00 80 00 00       	mov    $0x8000,%edx
 6b6:	81 ff ff 0f 00 00    	cmp    $0xfff,%edi
 6bc:	0f 46 da             	cmovbe %edx,%ebx
 6bf:	eb 10                	jmp    6d1 <malloc+0x61>
 6c1:	8d b4 26 00 00 00 00 	lea    0x0(%esi,%eiz,1),%esi
  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
 6c8:	8b 02                	mov    (%edx),%eax
    if(p->s.size >= nunits){
 6ca:	8b 48 04             	mov    0x4(%eax),%ecx
 6cd:	39 cf                	cmp    %ecx,%edi
 6cf:	76 3f                	jbe    710 <malloc+0xa0>
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    if(p == freep)
 6d1:	39 05 18 0a 00 00    	cmp    %eax,0xa18
 6d7:	89 c2                	mov    %eax,%edx
 6d9:	75 ed                	jne    6c8 <malloc+0x58>
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
 6db:	83 ec 0c             	sub    $0xc,%esp
 6de:	53                   	push   %ebx
 6df:	e8 96 fc ff ff       	call   37a <sbrk>
  if(p == (char*)-1)
 6e4:	83 c4 10             	add    $0x10,%esp
 6e7:	83 f8 ff             	cmp    $0xffffffff,%eax
 6ea:	74 1c                	je     708 <malloc+0x98>
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
 6ec:	89 70 04             	mov    %esi,0x4(%eax)
  free((void*)(hp + 1));
 6ef:	83 ec 0c             	sub    $0xc,%esp
 6f2:	83 c0 08             	add    $0x8,%eax
 6f5:	50                   	push   %eax
 6f6:	e8 e5 fe ff ff       	call   5e0 <free>
  return freep;
 6fb:	8b 15 18 0a 00 00    	mov    0xa18,%edx
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    if(p == freep)
      if((p = morecore(nunits)) == 0)
 701:	83 c4 10             	add    $0x10,%esp
 704:	85 d2                	test   %edx,%edx
 706:	75 c0                	jne    6c8 <malloc+0x58>
        return 0;
 708:	31 c0                	xor    %eax,%eax
 70a:	eb 1c                	jmp    728 <malloc+0xb8>
 70c:	8d 74 26 00          	lea    0x0(%esi,%eiz,1),%esi
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
 710:	39 cf                	cmp    %ecx,%edi
 712:	74 1c                	je     730 <malloc+0xc0>
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
 714:	29 f9                	sub    %edi,%ecx
 716:	89 48 04             	mov    %ecx,0x4(%eax)
        p += p->s.size;
 719:	8d 04 c8             	lea    (%eax,%ecx,8),%eax
        p->s.size = nunits;
 71c:	89 78 04             	mov    %edi,0x4(%eax)
      }
      freep = prevp;
 71f:	89 15 18 0a 00 00    	mov    %edx,0xa18
      return (void*)(p + 1);
 725:	83 c0 08             	add    $0x8,%eax
    }
    if(p == freep)
      if((p = morecore(nunits)) == 0)
        return 0;
  }
}
 728:	8d 65 f4             	lea    -0xc(%ebp),%esp
 72b:	5b                   	pop    %ebx
 72c:	5e                   	pop    %esi
 72d:	5f                   	pop    %edi
 72e:	5d                   	pop    %ebp
 72f:	c3                   	ret    
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
 730:	8b 08                	mov    (%eax),%ecx
 732:	89 0a                	mov    %ecx,(%edx)
 734:	eb e9                	jmp    71f <malloc+0xaf>
  Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
 736:	c7 05 18 0a 00 00 1c 	movl   $0xa1c,0xa18
 73d:	0a 00 00 
 740:	c7 05 1c 0a 00 00 1c 	movl   $0xa1c,0xa1c
 747:	0a 00 00 
    base.s.size = 0;
 74a:	b8 1c 0a 00 00       	mov    $0xa1c,%eax
 74f:	c7 05 20 0a 00 00 00 	movl   $0x0,0xa20
 756:	00 00 00 
 759:	e9 3e ff ff ff       	jmp    69c <malloc+0x2c>
