// Shell.

#include "types.h"
#include "user.h"
#include "fcntl.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10

struct cmd {
	int type;
};

struct execcmd {
	int type;
	//两种参数
	char *argv[MAXARGS];
	char *eargv[MAXARGS];
};

struct redircmd {
	int type;
	//重定向cmd
	struct cmd *cmd;
	//重定向文件
	char *file;
	//目标文件
	char *efile;
	int mode;
	int fd;
};

struct pipecmd {
	int type;
	//管道两侧需要递归解析新的cmd
	struct cmd *left;
	struct cmd *right;
};

struct listcmd {
	int type;
	//列表两个cmd
	struct cmd *left;
	struct cmd *right;
};

struct backcmd {
	int type;
	//后方的cmd
	struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
	int p[2];
	struct backcmd *bcmd;
	struct execcmd *ecmd;
	struct listcmd *lcmd;
	struct pipecmd *pcmd;
	struct redircmd *rcmd;

	if (cmd == 0)
		exit();

	//依据parse得到的cmd的类型选择将要使用的操作
	switch (cmd->type) {
	default:
		panic("runcmd");

	case EXEC:
		//先改变cmd的类型
		ecmd = (struct execcmd*)cmd;
		if (ecmd->argv[0] == 0)
			//无第一个参数(程序名)时退出
			exit();
		//按照参数启动程序
		exec(ecmd->argv[0], ecmd->argv);
		printf(2, "exec %s failed\n", ecmd->argv[0]);
		break;

	case REDIR:
		rcmd = (struct redircmd*)cmd;
		close(rcmd->fd);
		//打开重定向文件
		if (open(rcmd->file, rcmd->mode) < 0) {
			printf(2, "open %s failed\n", rcmd->file);
			exit();
		}
		//继续递归run
		runcmd(rcmd->cmd);
		break;

	case LIST:
		lcmd = (struct listcmd*)cmd;
		//类似之前shell里新建进程继续run
		if (fork1() == 0)
			runcmd(lcmd->left);
		//等待子进程完成run右边
		wait();
		runcmd(lcmd->right);
		break;

	case PIPE:
		//管道
		pcmd = (struct pipecmd*)cmd;
		//创建新管道失败时报错
		if (pipe(p) < 0)
			panic("pipe");
		if (fork1() == 0) {
			//在子进程中run管道左侧
			close(1);
			//dup复制文件描述符到p，然后关闭管道
			dup(p[1]);
			close(p[0]);
			close(p[1]);
			//run左边来继续读入
			runcmd(pcmd->left);
		}
		if (fork1() == 0) {
			//run管道右侧
			close(0);
			dup(p[0]);
			close(p[0]);
			close(p[1]);
			runcmd(pcmd->right);
		}
		//关闭管道并等待子进程结束
		close(p[0]);
		close(p[1]);
		wait();
		wait();
		break;

	case BACK:
		bcmd = (struct backcmd*)cmd;
		if (fork1() == 0)
			runcmd(bcmd->cmd);
		break;
	}
	exit();
}

int
getcmd(char *buf, int nbuf)
{
	//读取cmd的内容
	printf(2, "$ ");
	//初始化内存
	memset(buf, 0, nbuf);
	gets(buf, nbuf);
	//若第一个字符是结束符返回-1
	if (buf[0] == 0) // EOF
		return -1;
	return 0;
}

int
main(void)
{
	//shell的主程序
	static char buf[100];
	int fd;

	//确认三个文件描述符都开启
	// Ensure that three file descriptors are open.
	while ((fd = open("console", O_RDWR)) >= 0) {
		if (fd >= 3) {
			close(fd);
			break;
		}
	}

	// Read and run input commands.
	while (getcmd(buf, sizeof(buf)) >= 0) {
		if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
			//cd指令只能由父进程处理
			// Chdir must be called by the parent, not the child.
			//去除换行符
			buf[strlen(buf) - 1] = 0;  // chop \n
			if (chdir(buf + 3) < 0)
				//没有需要跳转的路径时打印
				printf(2, "cannot cd %s\n", buf + 3);
			continue;
		}
		if (fork1() == 0)
			//在子进程对其他的指令进行分析并run，父进程等待
			runcmd(parsecmd(buf));
		//wait等待处理
		wait();
	}
	exit();
}

void
panic(char *s)
{
	//panic打印内容并退出
	printf(2, "%s\n", s);
	exit();
}

int
fork1(void)
{
	//除了fork正确的返回外都引发panic
	int pid;

	//得到fork的pid
	pid = fork();
	if (pid == -1)
		//-1时为fork失败，打印错误提示并退出程序
		panic("fork");
	return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
	execcmd(void)
{
	struct execcmd *cmd;

	//此cmd是启动应用的类型
	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = EXEC;
	return (struct cmd*)cmd;
}

struct cmd*
	redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
	struct redircmd *cmd;

	//申请内存并初始化为0
	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = REDIR;
	cmd->cmd = subcmd;
	cmd->file = file;
	cmd->efile = efile;
	cmd->mode = mode;
	cmd->fd = fd;
	return (struct cmd*)cmd;
}

struct cmd*
	pipecmd(struct cmd *left, struct cmd *right)
{
	//将字符串得到的管道两端复制在cmd中
	struct pipecmd *cmd;

	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = PIPE;
	cmd->left = left;
	cmd->right = right;
	return (struct cmd*)cmd;
}

struct cmd*
	listcmd(struct cmd *left, struct cmd *right)
{
	struct listcmd *cmd;

	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = LIST;
	cmd->left = left;
	cmd->right = right;
	return (struct cmd*)cmd;
}

struct cmd*
	backcmd(struct cmd *subcmd)
{
	struct backcmd *cmd;

	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = BACK;
	cmd->cmd = subcmd;
	return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
	//效果是跳过token并返回遇到的token
	char *s;
	int ret;

	s = *ps;
	//找到第一个非空符
	while (s < es && strchr(whitespace, *s))
		s++;
	//指针非空时赋值这个字符，相当于返回实际找到的字符
	if (q)
		*q = s;
	//返回值为此字符的int
	ret = *s;
	switch (*s) {
	case 0:
		break;
	case '|':
	case '(':
	case ')':
	case ';':
	case '&':
	case '<':
		//当遇到的是token时，跳过
		s++;
		break;
	case '>':
		s++;
		//右尖括号也跳过，但如果接下来又是右尖括号，返回+
		if (*s == '>') {
			ret = '+';
			s++;
		}
		break;
	default:
		//其他符号返回a
		ret = 'a';
		//找到下个非空且非命令符的字符
		while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
			s++;
		break;
	}
	//若需要保存移动后的位置则保留在eq中
	if (eq)
		*eq = s;

	//读取下一个非空字符并定为新的子串起点
	while (s < es && strchr(whitespace, *s))
		s++;
	*ps = s;
	return ret;
}

int
peek(char **ps, char *es, char *toks)
{
	//ps：字符串开头，es：字符串尾，toks：分隔符
	//将实字符从字符流中取出
	char *s;

	//字符串的字符指针
	s = *ps;
	while (s < es && strchr(whitespace, *s))
		//当遇到空白类字符或字符串末尾时跳过
		s++;
	*ps = s;	//将位置赋值
	//返回值标定了取出的字符是否在想要的toks里
	return *s && strchr(toks, *s);
}

//parse：分析
struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

//分析控制台输入
struct cmd*
	parsecmd(char *s)
{
	char *es;
	struct cmd *cmd;

	es = s + strlen(s);	//得到字符串尾的地址
	cmd = parseline(&s, es);	//利用parseline来分析
	peek(&s, es, "");	//查看是否仍有剩余
	if (s != es) {	//若没到末尾(还有剩余)，代表格式错误
		printf(2, "leftovers: %s\n", s);
		panic("syntax");
	}
	//终止
	nulterminate(cmd);
	return cmd;
}

//分析多行
struct cmd*
	parseline(char **ps, char *es)
{
	struct cmd *cmd;

	cmd = parsepipe(ps, es);	//利用parsepipe来分析管道
	while (peek(ps, es, "&")) {
		gettoken(ps, es, 0, 0);
		cmd = backcmd(cmd);
	}
	if (peek(ps, es, ";")) {
		gettoken(ps, es, 0, 0);
		cmd = listcmd(cmd, parseline(ps, es));
	}
	return cmd;
}

//分析管道“|”
struct cmd*
	parsepipe(char **ps, char *es)
{
	struct cmd *cmd;

	cmd = parseexec(ps, es);	//利用parseexec来分析运行
	if (peek(ps, es, "|")) {
		gettoken(ps, es, 0, 0);
		cmd = pipecmd(cmd, parsepipe(ps, es));
	}
	return cmd;
}

//分析重定向“<,>,>>”
struct cmd*
	parseredirs(struct cmd *cmd, char **ps, char *es)
{
	int tok;
	char *q, *eq;

	while (peek(ps, es, "<>")) {
		tok = gettoken(ps, es, 0, 0);
		if (gettoken(ps, es, &q, &eq) != 'a')
			panic("missing file for redirection");
		switch (tok) {
		case '<':
			cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
			break;
		case '>':
			cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREATE, 1);
			break;
		case '+':  // >>
			//与>相同
			cmd = redircmd(cmd, q, eq, O_WRONLY | O_CREATE, 1);
			break;
		}
	}
	return cmd;
}

//分析块
struct cmd*
	parseblock(char **ps, char *es)
{
	struct cmd *cmd;

	//分析第一个非空字符是不是左括号
	if (!peek(ps, es, "("))
		panic("parseblock");
	//将刚才的左括号token取出
	gettoken(ps, es, 0, 0);
	//继续分析这一行
	cmd = parseline(ps, es);
	//分析是否有对应的右括号
	if (!peek(ps, es, ")"))
		panic("syntax - missing )");
	//取出右括号
	gettoken(ps, es, 0, 0);
	//分析是否重定向
	cmd = parseredirs(cmd, ps, es);
	return cmd;
}

//分析执行
struct cmd*
	parseexec(char **ps, char *es)
{
	char *q, *eq;
	int tok, argc;
	struct execcmd *cmd;
	struct cmd *ret;

	//若首个有效字符是左括号"("
	if (peek(ps, es, "("))
		return parseblock(ps, es);

	//创建cmd
	ret = execcmd();
	cmd = (struct execcmd*)ret;

	argc = 0;
	//分析重定向
	ret = parseredirs(ret, ps, es);
	while (!peek(ps, es, "|)&;")) {
		if ((tok = gettoken(ps, es, &q, &eq)) == 0)
			break;
		if (tok != 'a')
			panic("syntax");
		cmd->argv[argc] = q;
		cmd->eargv[argc] = eq;
		argc++;
		if (argc >= MAXARGS)
			panic("too many args");
		ret = parseredirs(ret, ps, es);
	}
	cmd->argv[argc] = 0;
	cmd->eargv[argc] = 0;
	return ret;
}

// NUL-terminate all the counted strings.
//递归终止分析出来的所有cmd
struct cmd*
	nulterminate(struct cmd *cmd)
{
	int i;
	struct backcmd *bcmd;
	struct execcmd *ecmd;
	struct listcmd *lcmd;
	struct pipecmd *pcmd;
	struct redircmd *rcmd;

	if (cmd == 0)
		return 0;

	//根据cmd的类型递归终止cmd
	switch (cmd->type) {
	case EXEC:
		ecmd = (struct execcmd*)cmd;
		//参数设0
		for (i = 0; ecmd->argv[i]; i++)
			*ecmd->eargv[i] = 0;
		break;

	case REDIR:
		rcmd = (struct redircmd*)cmd;
		nulterminate(rcmd->cmd);
		//efile设NULL
		*rcmd->efile = 0;
		break;

	//以下都是递归
	case PIPE:
		pcmd = (struct pipecmd*)cmd;
		nulterminate(pcmd->left);
		nulterminate(pcmd->right);
		break;

	case LIST:
		lcmd = (struct listcmd*)cmd;
		nulterminate(lcmd->left);
		nulterminate(lcmd->right);
		break;

	case BACK:
		bcmd = (struct backcmd*)cmd;
		nulterminate(bcmd->cmd);
		break;
	}
	return cmd;
}
