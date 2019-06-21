#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

// ************************************************************ const strings *
#define MAX_CHAR_INSHOW 80
#define BUF_LINE_LENGTH 512
#define LINE_IDX_SPACE 5
#define COLOR_SPACE 6
#define COMMAND_MAX_LEN 200
#define NULL 0
#define TRUE 1
#define FALSE 0

// ************************************************************* ansi control *
#define ESC "\033"
#define CLR ESC "[2J"
#define HOME ESC "[H"
#define REF CLR HOME

// ************************************************************** ansi colors *
#define BLK ESC "[30m"
#define RED ESC "[31m"
#define GRN ESC "[32m"
#define YEL ESC "[33m"
#define BLU ESC "[34m"
#define MAG ESC "[35m"
#define CYN ESC "[36m"
#define WHT ESC "[37m"

// *********************************************************** data structure *
// use linkedlist to store the text
typedef struct LineNode {
	struct LineNode* prev;
	int length;
	char* content;
	struct LineNode* next;
}LineNode;

// **************************************************** function decalrations *
int loadFile(char* filePath);
int saveFile(char* filePath);
void showFile(int idx, int lines);
void showHelp();
void cmdLoop();
int cmdExec(char* name, char* args);
int initList();
int freeList(LineNode* start, LineNode* end);
LineNode* newNode(LineNode* prev);
LineNode* deleteNode(LineNode* node);
LineNode* findNode(int index);

void ins(int idx, char* args);
void mod(int idx, char* args);
void num(char* args);
void del(char* args);
void cut(char* args);
int quit();

char* setStr(char* target, char* from, int len);
char* idx2Str(int idx);
int str2Idx(char* str);

// **************************************************************** variables *
int fileLineNum = 0;
int showLineIdx = 1;
LineNode* headNode;
LineNode* tailNode;
char* oriPath;
int isDirty = 0;
int headShowLine = 0;
int currentLine = 0;
LineNode* curNode;

// *********************************************************** function bodys *
int main(int argc, char* argv[])
{
	if (argc <= 1) {
		// if missing some parameters
		printf(1, "Please start this editor using [editor file_name]\n");
		exit();
		return -1;
	}

	if (loadFile(argv[1])) {
		// successfully check 
		printf(1, "successfully load! file lines: %d\n", fileLineNum);
	}
	else {
		// something goes wrong
		printf(1, "unsuccessfully load!\n\n");
		exit();
		return -1;
	}

	isDirty = FALSE;

	// show command help.
	showHelp();

	// reading user's commands
	cmdLoop();

	// editor exit, free the whole list
	freeList(headNode->next, tailNode->prev);
	printf(1, "free list completed!\n");
	free(headNode);
	free(tailNode);
	printf(1, "editor exit.\n");
	exit();

	return 0;
}

// load the file into first linkedlist
int loadFile(char* filePath) {
	// check if [filePath] is exist
	int file = open(filePath, O_RDONLY);
	if (file == -1) {
		printf(1, "file not found!\n");
		oriPath = filePath;
		// init it
		if (!initList()) {
			printf(1, "initList error!\n");
			return FALSE;
		}
		printf(1, "create new file!\n");
		return TRUE;
	}
	printf(1, "file found!\n");
	oriPath = filePath;

	// init it
	if (!initList()) {
		printf(1, "initList error!\n");
		return FALSE;
	}

	// the most important part
	// adding every lines into that linkedlist
	char buf[BUF_LINE_LENGTH] = {};
	int len = 0;
	int scaning = 0;
	LineNode* now = NULL;

	// read a new line
	while ((len = read(file, buf, BUF_LINE_LENGTH)) > 0) {
		// set two pointer here
		int start = 0;
		int end = 0;
		// if the [start pointer] is still inline
		while (start < len) {
			// find out the line's ending
			for (end = start; end < len && buf[end] != '\n'; ++end)
				;

			// [scaning == 1] means is still reading the latest line
			if (scaning == 0) {
				now = newNode(tailNode->prev);
			}

			if (now == NULL) {
				printf(1, "nowNode pointer is NULL!\n");
				return FALSE;
			}

			// if is a new line
			if (now->length == 0) {
				now->content = (char*)malloc(sizeof(char)*(0 + end - start));
				setStr(&now->content[0], &buf[start], end - start);
				now->length = 0 + end - start;
			}
			// if is a old line
			else {
				char* tempStr = now->content;
				now->content = (char*)malloc(sizeof(char)*(now->length
					+ end - start));
				setStr(&now->content[0], &tempStr[0], now->length);
				setStr(&now->content[now->length], &buf[start], end - start);
				now->length = now->length + end - start;
			}

			// if meets a line's ending
			if (buf[end] == '\n') {
				scaning = 0;
				now = NULL;
			}
			// this line is not end now
			else {
				scaning = 1;
			}

			// continue
			start = end + 1;
		}
	}

	close(file);
	return 1;
}

// save the file into harddisk
int saveFile(char * filePath)
{
	if (filePath[0] == '\0')
		filePath = oriPath;
	else
		oriPath = filePath;

	// delete old file
	unlink(filePath);

	int file = open(filePath, O_WRONLY | O_CREATE);
	if (file == -1)
	{
		printf(1, "save failed, file can't open: %s\n", filePath);
		return FALSE;
	}

	if (fileLineNum <= 0) {
		printf(1, "successfully saved! file lines: %d\n", fileLineNum);
		// refresh dirty flag
		isDirty = 0;
		return TRUE;
	}

	int count = 0;
	int part = fileLineNum / 10;
	LineNode* now = headNode->next;
	printf(1, "fileName: %s\n",filePath);
	printf(1, "saving");
	while (now != tailNode)
	{
		write(file, now->content, now->length);
		write(file, "\n", 1);
		now = now->next;
		++count;
		if (part>0&&count%part == 0) {
			printf(1, ".");
		}
	}

	close(file);

	printf(1, "saved! file lines: %d\n", count);
	// refresh dirty flag
	isDirty = 0;
	return TRUE;
}

// display file in commandline
void showFile(int index, int lines) {
	if (fileLineNum == 0) {
		printf(1, "the file is null!\n");
		return;
	}
	if (index > fileLineNum) {
		index = fileLineNum;
	}

	printf(1, "%s======================================================================================\n",REF);

	// init start node
	int endIndex;
	if (index == 0) {
		curNode = headNode->next;
		currentLine = 1;
		endIndex = fileLineNum;
		headShowLine = 1;
	}
	else {
		if (index < 0) {
			index = 1;
		}
		// backwards
		if (lines < 0) {
			headShowLine += lines;
			headShowLine -= 1;
			if (headShowLine < 0) {
				headShowLine = 1;
			}
			index=headShowLine;
		}
		LineNode* tmp = findNode(index);
		if (tmp == NULL) {
			return;
		}
		curNode = tmp;
		currentLine = index;
		headShowLine = index;
		// forwards
		if (lines < 0) {
			endIndex = currentLine - lines;
		}
		else {
			endIndex = currentLine + lines;
		}
		if (endIndex > fileLineNum) {
			endIndex = fileLineNum;
		}
	}

	char lineInShow[COLOR_SPACE+ LINE_IDX_SPACE + COLOR_SPACE-1 + MAX_CHAR_INSHOW + 2] = {};
	setStr(&lineInShow[0], "\033[32m", COLOR_SPACE);
	setStr(&lineInShow[COLOR_SPACE + LINE_IDX_SPACE], "\033[0m", COLOR_SPACE-1);

	while (curNode != tailNode)
	{
		// display one line
		int start = 0;
		while (start < curNode->length || start == 0) {
			int len = curNode->length - start;
			// modify length
			if (len > MAX_CHAR_INSHOW) {
				len = MAX_CHAR_INSHOW;
			}

			// should show line index
			if (showLineIdx) {
				setStr(&lineInShow[COLOR_SPACE + LINE_IDX_SPACE + COLOR_SPACE - 1], &curNode->content[start], len);
				char* idxS = NULL;
				// is true line
				if (start == 0) {
					idxS = idx2Str(currentLine);
					++currentLine;
				}
				// just part of a line
				else {
					idxS = idx2Str(-1);
				}
				setStr(&lineInShow[COLOR_SPACE], &idxS[0], LINE_IDX_SPACE);
				free(idxS);
				lineInShow[COLOR_SPACE + COLOR_SPACE - 1 + len + LINE_IDX_SPACE] = '\n';
				lineInShow[COLOR_SPACE + COLOR_SPACE - 1 + len + LINE_IDX_SPACE + 1] = '\0';
				// use write instead of printf
				write(1, lineInShow, COLOR_SPACE + COLOR_SPACE - 1+len + LINE_IDX_SPACE + 1);
			}
			// needn't show index
			else {
				setStr(&lineInShow[COLOR_SPACE+ COLOR_SPACE-1], &curNode->content[start], len);
				lineInShow[COLOR_SPACE + COLOR_SPACE - 1 + len] = '\n';
				lineInShow[COLOR_SPACE + COLOR_SPACE - 1 + len + 1] = '\0';
				// use write instead of printf
				write(1, lineInShow, COLOR_SPACE + COLOR_SPACE - 1 + len + 1);
			}

			if (len != 0)	
				start += len;
			else
				start += 1;
		}
		curNode = curNode->next;
		// jump out
		if (currentLine > endIndex) {
			break;
		}
	}
	printf(1, "======================================================================================\n");
}

// display help info
void showHelp()
{
	// use write instead of printf
	write(1,
		"\033[2J\033[H***************************************************************\n"
		"\033[0;32mPROGRAM INSTRUCTION:\n"
		"COMMANDS IN BRACKETS ARE NECESSARY VARIBLES\033[0m\n"
		"[i(ns) (string)]:            \033[0;32minsert (string) to the end.OK\033[0m\n"
		"[i(ns)- [idx] (string)]:     \033[0;32minsert (string) after line at (idx).OK\033[0m\n"
		"[d(el) (idx1) (idx2)]:       \033[0;32mdelete line from (idx1) to (idx2).OK\033[0m\n"
		"[m(od) (string)]:            \033[0;32mmodify the line at the end to (string).OK\033[0m\n"
		"[m(od)- [idx] (string)]:     \033[0;32mmodify the line at (idx) to (string).OK\033[0m\n"
		"[c(ut) [idx1] [idx2] [tar]]: \033[0;32mcut lines between (idx1) and (idx2) to (tar).OK\033[0m\n"
		"[l(ist) (num)]:              \033[0;32mdisplay 20 lines after num and set the cursor to (num+10).OK\033[0m\n"
		"[p(rev) (num)]:              \033[0;32mdisplay previous (num) lines and set the cursor after it.OK\033[0m\n"
		"[n(ext) (num)]:              \033[0;32mdisplay next (num) lines and set the cursor after it.OK\033[0m\n"
		"[num (1/0)]:                 \033[0;32mshow the line index or not.OK\033[0m\n"
		"[h(elp)]:                    \033[0;32mshow command help.OK\033[0m\n"
		"[w(ri) (path)]:              \033[0;32mwrite this file into (path).OK\033[0m\n"
		"[q(uit)]:                    \033[0;32mquit editor.OK\033[0m\n"
		"[wq]:                        \033[0;32mwrite this file into harddisk and quit editor.OK\033[0m\n"
		"***************************************************************\n",
		1331);

}

// analyze user's commands
void cmdLoop()
{
	// commands inputed
	char input[COMMAND_MAX_LEN] = {};
	char name[5] = {};
	char args[COMMAND_MAX_LEN - 5] = {};
	int cmdi = 0;
	int argi = 0;
	currentLine = 1;
	curNode = headNode->next;

	// an eternal loop for analyzing user's commands
	while (1)
	{
		// init
		if (isDirty) {
			printf(1, "\n*:");
		}
		else {
			printf(1, "\n :");
		}

		cmdi = 0;
		argi = 0;
		name[0] = '\0';
		args[0] = '\0';
		memset(input, 0, COMMAND_MAX_LEN);

		// get the input command
		gets(input, COMMAND_MAX_LEN);

		// get command name
		while (input[cmdi] != ' '&&input[cmdi] != '\n'&&input[cmdi] != '\0')
		{
			if (argi >= 4) {
				printf(1, "command name error!\n");
				goto NEXT;
			}
			name[argi] = input[cmdi];
			++argi;
			++cmdi;
		}
		name[argi] = '\0';
		if (input[cmdi] == '\n' || input[cmdi] == '\0') {
			goto EXEC;
		}

		++cmdi;
		argi = 0;

		// get command args
		while (input[cmdi] != '\n')
		{
			if (argi >= COMMAND_MAX_LEN - 1) {
				printf(1, "command args too long!\n");
				goto NEXT;
			}
			args[argi] = input[cmdi];
			++argi;
			++cmdi;
		}
		args[argi] = '\0';
		if (input[cmdi] == '\n')
			goto EXEC;
		else
			goto NEXT;

	EXEC:
		if (cmdExec(name, args)) {
			// return TRUE means should exit editor
			break;
		}
		continue;

	NEXT:
		// read next command
		continue;
	}
}

// try to exec the command
int cmdExec(char* name, char* args)
{
	if (name[0] == '\0') {
		printf(1, "get empty command!\n");
		return FALSE;
	}

	// insert line after 
	if (strcmp(name, "i") == 0 || strcmp(name, "ins") == 0) {
		ins(fileLineNum, args);
	}
	else if (strcmp(name, "i-") == 0 || strcmp(name, "ins-") == 0) {
		int idx = str2Idx(args);
		if (idx == 99999) {
			printf(1, "index inputed is wrong!\n");
			return 0;
		}
		int i = 0;
		for (i = 0; args[i] != ' '; ++i)
			;
		ins(idx, &args[i + 1]);
	}
	else if (strcmp(name, "d") == 0 || strcmp(name, "del") == 0) {
		del(args);
	}
	else if (strcmp(name, "m") == 0 || strcmp(name, "mod") == 0) {
		mod(fileLineNum, args);
	}
	else if (strcmp(name, "m-") == 0 || strcmp(name, "mod-") == 0) {
		int idx = str2Idx(args);
		if (idx == 99999) {
			printf(1, "index inputed is wrong!\n");
			return FALSE;
		}
		int i = 0;
		for (i = 0; args[i] != ' '; ++i)
			;
		mod(idx, &args[i + 1]);
	}
	else if (strcmp(name, "c") == 0 || strcmp(name, "cut") == 0) {
		cut(args);
	}
	else if (strcmp(name, "l") == 0 || strcmp(name, "list") == 0) {
		int idx = str2Idx(args);
		if (idx == 99999) {
			printf(1, "index inputed is wrong!\n");
			return FALSE;
		}
		if (idx != 0) {
			showFile(idx - 10, 20);
		}
		else {
			showFile(1, fileLineNum);
		}
	}
	else if (strcmp(name, "n") == 0 || strcmp(name, "next") == 0) {
		int lines = str2Idx(args);
		if (lines == 99999) {
			printf(1, "lines num inputed is wrong!\n");
			return FALSE;
		}
		if (lines <= 0) {
			lines = 20;
		}
		showFile(currentLine- lines/2, lines);
	}
	else if (strcmp(name, "p") == 0 || strcmp(name, "prev") == 0) {
		int lines = str2Idx(args);
		if (lines == 99999) {
			printf(1, "lines num inputed is wrong!\n");
			return FALSE;
		}
		if (lines <= 0) {
			lines = 20;
		}
		showFile(currentLine + lines / 2, -lines);
	}
	else if (strcmp(name, "num") == 0) {
		num(args);
	}
	else if (strcmp(name, "h") == 0 || strcmp(name, "help") == 0) {
		showHelp();
	}
	else if (strcmp(name, "w") == 0 || strcmp(name, "wri") == 0) {
		saveFile(args);
	}
	else if (strcmp(name, "q") == 0 || strcmp(name, "quit") == 0) {
		return quit();
	}
	else if (strcmp(name, "wq") == 0) {
		saveFile(args);
		return quit();
	}
	else {
		printf(1, "command not found!\n");
	}

	return FALSE;
}

// init linkedlist
int initList() {
	tailNode = (LineNode*)malloc(sizeof(LineNode));
	headNode = (LineNode*)malloc(sizeof(LineNode));
	if (tailNode == 0 || headNode == 0) {
		printf(1, "malloc error!\n");
		return 0;
	}

	tailNode->prev = headNode;
	tailNode->content = NULL;
	tailNode->length = 0;
	tailNode->next = NULL;

	headNode->prev = NULL;
	headNode->content = NULL;
	headNode->length = 0;
	headNode->next = tailNode;

	return 1;
}

// free the list from [start] to [end], cannot free root nodes
int freeList(LineNode* start, LineNode* end)
{
	if (start == headNode || end == tailNode) {
		printf(1, "cannot free list that contain root nodes!\n");
		return FALSE;
	}
	LineNode* now = start;
	LineNode* tmp = end->next;
	int delNum = 0;
	while (now != tmp)
	{
		now = deleteNode(now);
		if (now == NULL) {
			return FALSE;
		}
		++delNum;
	}
	return delNum;
}

// add a new node after [prev] and return it
LineNode* newNode(LineNode* prev) {
	if (prev == NULL) {
		printf(1, "the newNode's prev pointer is NULL!\n");
		return NULL;
	}
	LineNode* node;
	node = (LineNode*)malloc(sizeof(LineNode));

	if (node == NULL) {
		printf(1, "newNode alloc failed!\n");
		return NULL;
	}

	// insert new node into list
	LineNode* next = prev->next;
	if (next) {
		next->prev = node;
		prev->next = node;
		node->prev = prev;
		node->next = next;
	}
	else {
		prev->next = node;
		node->prev = prev;
		node->next = NULL;
	}
	node->content = NULL;
	node->length = 0;

	++fileLineNum;
	isDirty = 1;

	return node;
}

// delete this node and return the node after it, except root nodes
LineNode * deleteNode(LineNode * node)
{
	if (node == NULL) {
		printf(1, "node is NULL!\n");
		return NULL;
	}
	if (node == headNode || node == tailNode) {
		printf(1, "can't delete root nodes!\n");
		return NULL;
	}

	LineNode * next = node->next;
	node->prev->next = next;
	next->prev = node->prev;

	// free the select node
	node->next = NULL;
	node->prev = NULL;
	free(node->content);
	node->content = NULL;
	free(node);

	--fileLineNum;
	isDirty = 1;

	return next;
}

// return node in index
LineNode * findNode(int index)
{
	int lineIdx = 0;
	if (index > fileLineNum || index < 0) {
		printf(1, "index error!\n");
		return NULL;
	}
	LineNode* now = NULL;

	//between head and cur
	if (index <= currentLine) {
		// from head to cur
		if (index - 1 <= currentLine - index) {
			now = headNode->next;
			lineIdx = 1;
			while (now != curNode)
			{
				if (lineIdx == index) {
					break;
				}
				now = now->next;
				++lineIdx;
			}
		}
		// from cur to head
		else {
			now = curNode;
			lineIdx = currentLine;
			while (now->prev != headNode)
			{
				if (lineIdx == index) {
					break;
				}
				now = now->prev;
				--lineIdx;
			}
		}
	}
	//between cur and tail
	else {
		// from cur to tail
		if (index - currentLine <= fileLineNum - index) {
			now = curNode;
			lineIdx = currentLine;
			while (now->next != tailNode)
			{
				if (lineIdx == index) {
					break;
				}
				now = now->next;
				++lineIdx;
			}
		}
		// from tail to cur
		else {
			now = tailNode->prev;
			lineIdx = fileLineNum;
			while (now != curNode)
			{
				if (lineIdx == index) {
					break;
				}
				now = now->prev;
				--lineIdx;
			}
		}
	}
	return now;
}

// insert a line
void ins(int idx, char * args)
{
	LineNode* prev;
	if (idx == 0)
		prev = headNode;
	else
		prev = findNode(idx);
	if (prev == NULL) {
		return;
	}

	int len = 0;

	// ins idx string
	for (len = 0; args[len] != '\0' && args[len] != '\n'; ++len)
		;

	LineNode* now = newNode(prev);
	now->length = len;
	now->content = (char*)malloc(sizeof(char)*len);
	setStr(&now->content[0], &args[0], len);

	printf(1, "\033[32m%s\033[0m%s\n", idx2Str(idx + 1), now->content);
}

// modify lines
void mod(int idx, char * args)
{
	LineNode* prev;
	if (idx == 0) {
		printf(1, "index inputed is wrong!\n");
	}
	else
		prev = findNode(idx)->prev;
	if (prev == NULL) {
		return;
	}

	int len = 0;

	// ins idx string
	for (len = 0; args[len] != '\0' && args[len] != '\n'; ++len)
		;

	// delete and new
	deleteNode(prev->next);
	LineNode* now = newNode(prev);
	now->length = len;
	now->content = (char*)malloc(sizeof(char)*len);
	setStr(&now->content[0], &args[0], len);

	printf(1, "\033[32m%s\033[0m%s\n", idx2Str(idx), now->content);
}

// line index function
void num(char * args)
{
	if (args[0] == '\0') {
		if (showLineIdx) {
			showLineIdx = 0;
			printf(1, "stop showing line index.\n");
		}
		else {
			showLineIdx = 1;
			printf(1, "start showing line index.\n");
		}
		return;
	}
	if (args[1] == '\0') {
		if (args[0] == 0) {
			showLineIdx = 0;
			printf(1, "stop showing line index.\n");
		}
		else if (args[0] == 1) {
			showLineIdx = 1;
			printf(1, "start showing line index.\n");
		}
		else {
			printf(1, "show: 1 ; stop: 0\n");
		}
	}
	else {
		printf(1, "show: 1 ; stop: 0\n");
	}
}

// delete the lines in range
void del(char * args)
{
	// get index
	int idx1 = 0, idx2 = 0;
	// last line
	if (args[0] == '\0') {
		idx1 = fileLineNum;
		idx2 = fileLineNum;
	}
	// target line
	else {
		idx1 = str2Idx(args);
		if (idx1 == 99999) {
			printf(1, "index1 inputed is wrong!\n");
			return;
		}
		int i = 0;
		for (i = 0; args[i] != ' ' && args[i] != '\n' && args[i] != '\0'&&i < COMMAND_MAX_LEN; ++i)
			;
		if (args[i] != ' ' && args[i] != '\n' && args[i] != '\0') {
			printf(1, "index2 inputed is wrong!\n");
			return;
		}
		if (args[i] != ' ') {
			idx2 = idx1;
		}
		// line block
		else {
			idx2 = str2Idx(&args[i + 1]);
			if (idx2 == 99999) {
				printf(1, "index2 inputed is wrong!\n");
				return;
			}
		}
	}

	if (idx1 <= idx2) {
		int delNum = freeList(findNode(idx1), findNode(idx2));
		if (delNum > 0)
			printf(1, "%d - %d deleted. %d lines. fileLine: %d\n", idx1, idx2, delNum, fileLineNum);
		else
			printf(1, "delete %d - %d error.\n", idx1, idx2);
	}
	else {
		printf(1, "idx2 should larger than idx1!\n");
	}
}

// cutting block [idx1,idx2] and link them after [tar]
void cut(char * args)
{
	// get index
	int idx1 = 0, idx2 = 0, tar = 0;

	if (args[0] == '\0') {
		printf(1, "index1 inputed is wrong!\n");
		return;
	}
	// target line
	else {
		idx1 = str2Idx(args);
		if (idx1 == 99999) {
			printf(1, "index1 inputed is wrong!\n");
			return;
		}
		int i = 0;
		for (i = 0; args[i] != ' ' && args[i] != '\n' && args[i] != '\0'&&i < COMMAND_MAX_LEN; ++i)
			;
		if (args[i] != ' ') {
			printf(1, "index2 inputed is wrong!\n");
			return;
		}
		// line block
		else {
			++i;
			idx2 = str2Idx(&args[i]);
			if (idx2 == 99999) {
				printf(1, "index2 inputed is wrong!\n");
				return;
			}
			for (i; args[i] != ' ' && args[i] != '\n' && args[i] != '\0'&&i < COMMAND_MAX_LEN; ++i)
				;
			if (args[i] != ' ') {
				printf(1, "tar inputed is wrong!\n");
				return;
			}
			// line block
			else {
				tar = str2Idx(&args[i + 1]);
				if (tar == 99999) {
					printf(1, "tar inputed is wrong!\n");
					return;
				}
			}
		}
	}

	if (idx1 <= idx2 && (tar<idx1 || tar>idx2)) {
		// parse lines block
		LineNode* nidx1 = findNode(idx1);
		LineNode* nidx2 = findNode(idx2);
		LineNode* ntar = findNode(tar);
		// link together
		nidx1->prev->next = nidx2->next;
		nidx2->next->prev = nidx1->prev;
		ntar->next->prev = nidx2;
		nidx2->next = ntar->next;
		ntar->next = nidx1;
		nidx1->prev = ntar;
		printf(1, "successfully linked\n");
	}
	else {
		printf(1, "index illegal!\n");
	}
}

// quit the editor
int quit()
{
	if (isDirty) {
		printf(1, "this file has being changed! do you want to overwrite?(Y/N)\n");
		char input[COMMAND_MAX_LEN] = {};
		gets(input, COMMAND_MAX_LEN);
		if (input[1] != '\0'&&input[1] != '\n') {
			printf(1, "input error!\n");
			return FALSE;
		}
		if (input[0] == 'Y' || input[0] == 'y') {
			saveFile(oriPath);
			return TRUE;
		}
		else if (input[0] == 'N' || input[0] == 'n')
			return TRUE;
		else {
			printf(1, "input error!\n");
			return FALSE;
		}
	}
	else {
		return TRUE;
	}
}

// parse [from+len] to target, return target itself
char* setStr(char* target, char* from, int len) {
	int i = 0;
	for (i = 0; i < len; ++i) {
		target[i] = from[i];
	}

	return target;
}

// convert int to str
char * idx2Str(int idx)
{
	char* str = (char*)malloc(sizeof(char)*LINE_IDX_SPACE);
	if (str == NULL) {
		printf(1, "idx string alloc failed\n");
		return NULL;
	}

	// get the mod number
	int i = 0, mod = 1;
	if (idx != -1) {
		for (i = 0; i < LINE_IDX_SPACE - 1; ++i) {
			mod *= 10;
		}
		while (idx%mod == 0) {
			mod /= 10;
		}
		mod /= 10;
	}
	else {
		// space string
		mod = 0;
	}

	// fill str
	for (i = 0; i < LINE_IDX_SPACE; ++i) {
		if (mod >= 1) {
			str[i] = 48 + (idx / mod) % 10;
			mod /= 10;
		}
		else {
			str[i] = ' ';
		}
	}
	return str;
}

// need the index wrote in the begin of the str and splited with a space
int str2Idx(char * str)
{
	int idx = 0, i = 0;
	for (i = 0; i < LINE_IDX_SPACE; ++i) {
		if (str[i] == ' ' || str[i] == '\0' || str[i] == '\n')
			break;
		if (str[i] >= '0'&&str[i] <= '9') {
			idx *= 10;
			idx += str[i] - '0';
		}
	}
	if (str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
		idx = 99999;
	return idx;
}
