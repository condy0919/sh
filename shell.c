/**
 * @file   shell.c
 * @author  <condy@Arch>
 * @date   Mon Aug  5 14:25:34 2013
 * 
 * @brief  a little sh
 *         sh, receive command from stdin.
 *         sh -t, similar with top one except only run once.
 *         sh -c command, run the command once.
 *         only these 3 modes.
 */



#include "shell.h"

tree_node_t* token[TOKEN_NUMBER];

int main(int argc, char* argv[])
{
    int opt;
    char* cmd = NULL;
    int flag = 0;
    
    
    initialize_readline();// init command completion
    walk_bin_dir();// create command list

    while ((opt = getopt(argc, argv, "tc:")) != -1)
	switch (opt) {
	case 't':
	    flag |= 0x1;
	    break;
	case 'c':
	    flag |= 0x2;
	    cmd = optarg;
	    break;
	default:
	    fprintf(stderr, "unknown parameter\n");
	    exit(1);
	}

    if (flag == 3) {
	fprintf(stderr, "error option.\n");
	destroy_cmd_list();
	exit(1);
    } else if (flag == 2 || flag == 1) {// c mode or t mode
	cmd = (flag == 2) ? argv[2] : getl();

	int n = cope(cmd, token, TOKEN_NUMBER);

	tree_node_t* root = syntax_tree(token, n);
	do_analyse(root);
	execute(root, NULL, NULL);
	
	if (flag == 1)
	    free(cmd);
	destroy_token(root);	
	destroy_cmd_list();
	exit(0);
    }

    while ((cmd = getl())) {
//	printf("print is %s\n", cmd);
	int n = cope(cmd, token, TOKEN_NUMBER);
	free(cmd);

	/*
	  parse and build syntax tree
	  then execute code.
	*/
	if (n > 0) {
	    tree_node_t* root = syntax_tree(token, n);
	    do_analyse(root);
	    execute(root, NULL, NULL);
	    
	    destroy_token(root);
	}
	memset(token, 0, sizeof(token));
    }

    // recycle memory used for bin_cmd_list
    destroy_cmd_list();
    exit(0);
}


int cope(char* str, tree_node_t* array[], int n)
{
    int i = 0;
    int t;

    char* front;
    char* end;

    front = next_command(str);
    end = next_filter(front);

    do {
	// [front, end)
	i = split(array, i, front, end);
	t = array[i - 1]->part_cmd.cmd_size;
	array[i - 1]->part_cmd.cmd[t] = '\0';
	
	if (i == -1)
	    return -1;
	
	if (*end) {
	    i = split(array, i, end, end + 1);// split the filter
	    t = array[i - 1]->part_cmd.cmd_size;
	    array[i - 1]->part_cmd.cmd[t] = '\0';	   

	    if (i == -1)
		return -1;
	}
	front = next_command(end);
	end = next_filter(front);
    } while (end && i < n);

    return i;
}



int split(tree_node_t* array[], int i, char* front, char* end)
{
    char buffer[1024] = { 0 };
    char* p = buffer;
    char* sub;
    int n;

    if (front >= end)
	return i;
    
    if (*(end - 1) == ' ')
	--end;

    if (array[i] == NULL) {
	array[i] = malloc(sizeof(tree_node_t));
	init_tree_node(array[i]);
    }

    // set property
    if (end - front == 1) {// |;&
	switch (*front) {
	case '|':
	    array[i]->type = TFIL;
	    break;
	case ';':
	    array[i]->type = TLST;
	    break;
	case '&':
	    array[i]->type = TLST;
	    break;
	case '>':
	case '<':
	    break;
	    
	default:
	    fprintf(stderr, "error occoured.\n");
	    return -1;
	}
    } else
	array[i]->type = TCOM;
    
    memcpy(buffer, front, end - front);
    // spilt the > or < command
    while ((sub = strsep(&p, " ")) != NULL) {
	char* s;
	if ((s = strstr(sub, "<"))) {
	    split(array, i, sub, s);
	    
	    n = array[i]->part_cmd.cmd_size++;
	    char temp[2] = { *s, '\0' };
	    array[i]->part_cmd.cmd[n] = strdup(temp);
	    
	    split(array, i, s + 1, sub + strlen(sub));
	} else if ((s = strstr(sub, ">>"))) {
	    split(array, i, sub, s);
	    
	    n = array[i]->part_cmd.cmd_size++;
	    char temp[3] = { *s, *s, '\0' };
	    array[i]->part_cmd.cmd[n] = strdup(temp);
	    
	    split(array, i, s + 2, sub + strlen(sub));
	} else if ((s = strstr(sub, ">"))) {
	    split(array, i, sub, s);
	    
	    n = array[i]->part_cmd.cmd_size++;
	    char temp[2] = { *s, '\0' };
	    array[i]->part_cmd.cmd[n] = strdup(temp);

	    split(array, i, s + 1, s + strlen(sub));		  
	} else {
	    n = array[i]->part_cmd.cmd_size++;
	    array[i]->part_cmd.cmd[n] = strdup(sub);
	}
    }

    return i + 1;
}


void destroy_token(tree_node_t* root)
{
    int i;

    if (root->left)
	destroy_token(root->left);
    if (root->right)
	destroy_token(root->right);

    for (i = 0; i < root->part_cmd.cmd_size; ++i)
	free(root->part_cmd.cmd[i]);
    free(root);
}


tree_node_t* syntax_tree(tree_node_t* token[], int n)
{
    int i;

    for (i = 1; i < n; i += 2) {
	token[i]->left = token[i - 1];
	token[i]->right = token[i + 1];
	token[i + 1] = token[i];
    }

    return token[n - 1];
}


// for redirection
void do_analyse(tree_node_t* root)
{
    if (!root)
	return;
    
    if (root->type == TCOM) {
	int i;

	for (i = 0; i < root->part_cmd.cmd_size; ++i) {
	    int flag = 0;
	    
	    if (strcmp(root->part_cmd.cmd[i], ">") == 0) {
		root->redirect_out = creat(
		    root->part_cmd.cmd[i + 1], 0644);
		assert(root->redirect_out != -1);
		flag = 1;
	    } else if (strcmp(root->part_cmd.cmd[i], "<") == 0) {
		root->redirect_in = open(
		    root->part_cmd.cmd[i], O_RDONLY);
		assert(root->redirect_in != -1);
		flag = 1;
	    } else if (strcmp(root->part_cmd.cmd[i], ">>") == 0) {
		root->redirect_out = open(
		    root->part_cmd.cmd[i], O_WRONLY | O_APPEND | O_CREAT);
		assert(root->redirect_out);
		flag = 1;
	    }

	    if (flag != 0) {
		int j = i;
		root->part_cmd.cmd[i] = NULL;		    
		for (i += 2; root->part_cmd.cmd[i]; ++i, ++j)
		    root->part_cmd.cmd[j] = root->part_cmd.cmd[i];
	    }
	}// find the command redirection and open the file.
    }

    do_analyse(root->left);
    do_analyse(root->right);
}


void execute(tree_node_t* root, int fp1[], int fp2[])
{
    int pv[2];

    if (!root)
	return;

    switch (root->type) {
    case TCOM:
	// built-in command
	if (equal(root->part_cmd.cmd[0], "cd")) {
	    assert(root->part_cmd.cmd[1]);
	    chdir(root->part_cmd.cmd[1]);
	    return;
	} else if (equal(root->part_cmd.cmd[0], "pwd")) {
	    char buffer[512];
	    getcwd(buffer, sizeof(buffer));
	    fprintf(stderr, "%s\n", buffer);
	    return;
	} else if (equal(root->part_cmd.cmd[0], "exit")) {
	    exit(0);
	}

	pid_t pid;

	pid = fork();

	if (pid < 0) {
	    perror("fork");
	    exit(1);
	} else if (pid != 0) {
	    // close some resources and wait child process
	    if (root->flag.f_pin) {
		close(fp1[0]);
		close(fp1[1]);
	    }


	    // last command, wait child process
	    //if (!root->flag.f_pout)
		wait(NULL);
	    return;
	}

	// redirect to stdin
	if (root->redirect_in != -1) {
	    dup2(root->redirect_in, 0);
	    close(root->redirect_in);
	}
	// redirect to stdout
	if (root->redirect_out != -1) {
	    dup2(root->redirect_out, 1);
	    close(root->redirect_out);
	}

	// pipe
	if (root->flag.f_pin) {
	    dup2(fp1[0], 0);
	    close(fp1[0]);
	    close(fp1[1]);
	}
	if (root->flag.f_pout) {
	    dup2(fp2[1], 1);
	    close(fp2[1]);
	    close(fp2[0]);
	}

	// exec the command
	execvp(root->part_cmd.cmd[0], root->part_cmd.cmd);
	perror("execvp");
	exit(-1);
	
	break;
	
    case TLST:// & and ;
	execute(root->left, NULL, NULL);
	execute(root->right, NULL, NULL);
	break;
	
    case TFIL:// |
	pipe(pv);// create a pipe.

	// left node must write data to pipe.
	root->left->flag.f_pout = 1;
	root->left->flag.f_pin = root->flag.f_pin;// inherit
	execute(root->left, fp1, pv);

	// right node must read date from pipe
	root->right->flag.f_pin = 1;
	root->right->flag.f_pout = root->flag.f_pout;// inherit
	execute(root->right, pv, fp2);
	break;
    }
}











