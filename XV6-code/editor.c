#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

// ************************************************************ const strings *
#define MAX_CHAR_INSHOW 80
#define BUF_LINE_LENGTH 320
#define LINE_IDX_SPACE 5
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
void showFile();
void showHelp();
void cmdLoop();
int initList();
int freeList();
LineNode* newNode(LineNode* prev);
LineNode* deleteNode(LineNode* node);
LineNode* findNode(int index);
char* setStr(char* target, char* from, int len);
char* idx2Str(int idx);

// **************************************************************** variables *
int fileLineNum = 0;
int showLineIdx = 0;
LineNode* headNode;
LineNode* tailNode;
int latestLine;
LineNode* latestNode;

// *********************************************************** function bodys *
int main(int argc, char* argv[])
{
	if (argc == 1) {
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

	// display all this file
	showFile();

	// show command help.
	showHelp();

	// reading user's commands
	cmdLoop();

	// editor ends normally
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
				fileLineNum++;
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
	printf(1, "\n================================================================================\n");
	LineNode* now = headNode->next;
	char lineInShow[LINE_IDX_SPACE + MAX_CHAR_INSHOW + 1] = {};
	int lineIdx = 0;
	while (now != tailNode)
	{
		// display one line
		int start = 0;
		while (start < now->length||start==0) {
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

void showHelp()
{
	printf(1, "********************************************************************************\n");
	printf(1, "PROGRAM HELP:\n");
	printf(1, "[v]: display all this file.\n");
	printf(1, "[V]: display all this file with line index.\n");
	printf(1, "[n [num]]: change line's width\n");
	printf(1, "[K]: show command help.\n");
	printf(1, "[w]: save the file\n");
	printf(1, "[q]: quit editor\n");
	printf(1, "********************************************************************************\n");
}

// reading user's commands
void cmdLoop()
{
	// character inputed
	char c;

	// an eternal loop for reading user's commands
	while (1)
	{
		// read from the standard input
		if (read(0, &c, 1) < 1)
			continue;
		// help
		if (c == 'K') {
			showHelp();
		}
		// show file normally
		else if (c == 'v') {
			showLineIdx = 0;
			showFile();
		}
		// show file with index
		else if (c == 'V') {
			showLineIdx = 1;
			showFile();
		}
		// esc
		else if (c == 'q') {
			break;
		}
		// input mode
		else if (c == 'i') {
			continue;
		}
		else if (c == '\n') {
			continue;
		}
		else {
			printf(1, "command error\n");
		}
	}
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

int freeList()
{
	LineNode* now = headNode->next;
	while (now != tailNode)
	{
		now = deleteNode(now);
		if (now == NULL) {

		}
	}
	printf(1, "free list completed!\n");
	return 1;
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

	return node;
}

// delete this node and return the node after it
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
	free(node->content);
	free(node);

	return next;
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
