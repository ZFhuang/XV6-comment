#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

// ************************************************************ const strings *
#define MAX_CHAR_INSHOW 80
#define BUF_LINE_LENGTH 320
#define LINE_IDX_SPACE 5
#define COMMAND_MAX_LEN 100
#define NULL 0

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
void showFile();
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

char* setStr(char* target, char* from, int len);
char* idx2Str(int idx);
int str2Idx(char* str);

// **************************************************************** variables *
int fileLineNum = 0;
int showLineIdx = 1;
LineNode* headNode;
LineNode* tailNode;
int isDirty = 0;
char* path;
//int curIdx;
//LineNode* curNode;

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
		// check successfully
		printf(1, "Load successfully! File lines: %d\n", fileLineNum);
	}
	else {
		// something goes wrong
		printf(1, "Load unsuccessfully!\n\n");
		exit();
		return -1;
	}

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
		printf(1, "File not found!\n");
		return 0;
	}
	printf(1, "File found!\n");
	path = filePath;

	// init it
	if (!initList()) {
		printf(1, "initList error!\n");
		return 0;
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
				return 0;
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

	return 1;
}

// display file in commandline
void showFile() {
	if (fileLineNum == 0) {
		printf(1,"the file is null!\n");
		return;
	}

	printf(1, "\n================================================================================\n");
	LineNode* now = headNode->next;
	char lineInShow[LINE_IDX_SPACE + MAX_CHAR_INSHOW + 1] = {};
	int lineIdx = 0;
	while (now != tailNode)
	{
		// display one line
		int start = 0;
		while (start < now->length || start == 0) {
			int len = now->length - start;
			// modify length
			if (len > MAX_CHAR_INSHOW) {
				len = MAX_CHAR_INSHOW;
			}

			// should show line index
			if (showLineIdx) {
				setStr(&lineInShow[LINE_IDX_SPACE], &now->content[start], len);
				char* idxS = NULL;
				// is true line
				if (start == 0) {
					++lineIdx;
					idxS = idx2Str(lineIdx);
				}
				// just part of a line
				else {
					idxS = idx2Str(-1);
				}
				setStr(&lineInShow[0], &idxS[0], LINE_IDX_SPACE);
				free(idxS);
				lineInShow[len + LINE_IDX_SPACE] = '\0';
			}
			// needn't show index
			else {
				setStr(&lineInShow[0], &now->content[start], len);
				lineInShow[len] = '\0';
			}

			printf(1, "%s\n", lineInShow);
			if (len != 0)
				start += len;
			else
				start += 1;
		}
		now = now->next;
	}
	printf(1, "\n================================================================================\n");
}

// display help info
void showHelp()
{
	printf(1, "********************************************************************************\n");
	printf(1, "PROGRAM INSTRUCTION:\n");

	printf(1, "[i(ns) (string)]: insert (string) to the end.OK\n");
	printf(1, "[i(ns)- [idx] (string)]: insert (string) after line at (idx).OK\n");

	printf(1, "[d(el) (idx1) (idx2)]: delete line from (idx1) to (idx2).OK\n");

	printf(1, "[m(od) (string)]: modify the line at the end to (string).\n");
	printf(1, "[m(od)- [idx] (string)]: modify the line at (idx) to (string).\n");

	printf(1, "[l(ist)]: display all this file.OK\n");

	printf(1, "[n(um) (1/0)]: show the line index or not.OK\n");

	printf(1, "[h(elp)]: show command help.OK\n");

	printf(1, "[w(ri)]: write this file into harddisk.\n");

	printf(1, "[q(uit)]: quit editor.OK\n");

	printf(1, "[wq]: write this file into harddisk and quit editor.\n");
	printf(1, "********************************************************************************\n");
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

	// an eternal loop for analyzing user's commands
	while (1)
	{
		// init
		printf(1, ":");
		cmdi = 0;
		argi = 0;
		name[0] = '\0';
		args[0] = '\0';

		// get the input command
		gets(input, COMMAND_MAX_LEN);

		// get command name
		while (input[cmdi] != ' '&&input[cmdi] != '\n')
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
		if (input[cmdi] == '\n') {
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
		if (input[cmdi] == '\n') {
			goto EXEC;
		}

	EXEC:
		if (cmdExec(name, args)) {
			// return 1 means should exit editor
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
		return 0;
	}

	// insert line after 
	if (strcmp(name, "i") == 0 || strcmp(name, "ins") == 0) {
		ins(fileLineNum, args);
	}
	else if (strcmp(name, "i-") == 0 || strcmp(name, "ins-") == 0) {
		int idx = str2Idx(args);
		if (idx == -1) {
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
	if (strcmp(name, "m") == 0 || strcmp(name, "mod") == 0) {
		mod(fileLineNum, args);
	}
	else if (strcmp(name, "m-") == 0 || strcmp(name, "mod-") == 0) {
		int idx = str2Idx(args);
		if (idx == -1) {
			printf(1, "index inputed is wrong!\n");
			return 0;
		}
		int i = 0;
		for (i = 0; args[i] != ' '; ++i)
			;
		mod(idx, &args[i + 1]);
	}
	else if (strcmp(name, "l") == 0 || strcmp(name, "list") == 0) {
		showFile();
	}
	else if (strcmp(name, "n") == 0 || strcmp(name, "num") == 0) {
		num(args);
	}
	else if (strcmp(name, "h") == 0 || strcmp(name, "help") == 0) {
		showHelp();
	}
	else if (strcmp(name, "q") == 0 || strcmp(name, "quit") == 0) {
		return 1;
	}
	else {
		printf(1, "command not found!\n");
	}

	return 0;
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
		return 0;
	}
	LineNode* now = start;
	LineNode* tmp = end->next;
	int delNum = 0;
	while (now != tmp)
	{
		now = deleteNode(now);
		if (now == NULL) {
			return 0;
		}
		++delNum;
	}
	fileLineNum -= delNum;
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
	fileLineNum++;

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

	// from head to tail
	if (index - 1 <= fileLineNum - index) {
		now = headNode->next;
		lineIdx = 1;
		while (now->next != tailNode)
		{
			if (lineIdx == index) {
				break;
			}
			now = now->next;
			++lineIdx;
		}

	}
	// from tail to head
	else {
		now = tailNode->prev;
		lineIdx = fileLineNum;
		while (now->prev != headNode)
		{
			if (lineIdx == index) {
				break;
			}
			now = now->prev;
			--lineIdx;
		}
	}
	return now;
}

// insert function
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

	printf(1, "%s%s\n", idx2Str(idx + 1), now->content);
	isDirty = 1;
}

void mod(int idx, char * args)
{
	LineNode* prev;
	if (idx == 0){
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

	deleteNode(prev->next);
	LineNode* now = newNode(prev);
	now->length = len;
	now->content = (char*)malloc(sizeof(char)*len);
	setStr(&now->content[0], &args[0], len);

	printf(1, "%s%s\n", idx2Str(idx), now->content);
	isDirty = 1;
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
		if (idx1 == -1) {
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
			if (idx2 == -1) {
				printf(1, "index2 inputed is wrong!\n");
				return;
			}
		}
	}

	if (idx1 <= idx2) {
		int delNum = freeList(findNode(idx1), findNode(idx2));
		if (delNum > 0)
			printf(1, "%d - %d deleted. %d lines.\n", idx1, idx2, delNum);
		else
			printf(1, "delete %d - %d error.\n", idx1, idx2);
	}
	else {
		printf(1, "idx2 should larger than idx1!\n");
	}

	isDirty = 1;
}

// parse [from+len] to target
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
		idx *= 10;
		idx += str[i] - '0';
	}
	if (str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
		idx = -1;
	return idx;
}
