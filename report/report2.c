#include<stdio.h>
#include<stdlib.h>
#include<time.h>

/*�@�v���O�����̎��s�̎d��
 *�@./report2 sample10x10.pzl
 *�@�̗l�ɁA���s�t�@�C���̈����Ƀp�Y���̃t�@�C������n���Ď��s����
 */


//�̎�̏����i�[����\����(s�͌��݂̖ʐρAn�͎��g�̐����Ais_closed�͕����Ă��邩�ǂ���)
typedef struct{ int id, n, x, y, s, is_closed; }Master;
//���D��T���p�Ƀ}�X�̍��W�y�эŒZ���B�l���i�[����\����
typedef struct{ int x, y, n; }Masu;
//�S�񋓒T���p�ɗ񋓐��̏����i�[����\����(condition�́A�Ō�ɍs��ꂽ�񋓂ɂ�������(KABE or SPACE))
typedef struct{	int kabe, slave, condition; }Candidate;
//����T���p�ɉ���̐[���y�щ���O�̏�Ԃ��i�[����\����(dep�͐[���Apast_state�͉���O�̃}�X�̏��(SPACE or NORA)is_parent�͉���̐e�ł��邩�ǂ���)
typedef struct{	int dep, past_state, past_dep, is_parent; }ASS;


/*:::::�O���[�o���ϐ��S:::::*/

//�S�񋓂̌��}�X�̐��̏���̂������l
int T = 15;

//�S�̂̏��
int state_of_program;
//�T�C�Y�i�[
int X, Y;

//�萔
const int KABE = -1;
const int SPACE = 0;
const int SLAVE = 10;
const int NORA = -2;
const int MADA = -1;//���B�_�𒲂ׂ�ۂɎg��
const int DEL = -2;//���B�_�𒲂ׂ�ۂɎg��
const int MASTER = 2;//��������Ŏg��
const int SOLVING = 0;//�񓚒�
const int CLEAR = 1;//���������܂���

//�}�b�v�i�[ [X][Y]
char c_map[36][36];//�t�@�C������̎󂯎��p
int map[36][36];//���C���}�b�v(0=SPACE, 1-9=MASTER, 10=SLAVE)
int master_map[36][36];//�̎�}�b�v(���̗̒n�̗̎��id���L�^���Ă����B�������A�̎喢�m��̗̒n(�m���̒n)�͒lNORA�����B
int check[36][36];//�`�F�b�N�z��B
int check2[36][36];//��2�`�F�b�N�z��Bchec[][]�����ł͕s���̍ۂɎg�p
int bfs_check[36][36];//���D��T���p�B�ő�c��������L�^
int bfs_at_first_check[36][36];//���D��T����p���ē��B�_�𒲂ׂ�ہA���̃}�X�ɓ��B�����̎���L�^���Ă���(�܂����B���Ă��Ȃ�=MADA�A��l�ȏ㓞�B����=DEL�A��l�������B����=id)
int unreachable_check[36][36];//���B�s�\�ȃ}�X���`�F�b�N���Ă���
Candidate check_cand[36][36];//���J�E���g�p

//�l�����z��
int directX[] = {0, 1, 0,-1};
int directY[] = {1, 0,-1, 0};
//�E���l�����z��
int direct2X[] = {0, 1, 0, 1};
int direct2Y[] = {0, 0, 1, 1};

//�̎僊�X�g
int M;
Master master[200];

//�ǂ̐�
int now_w;
int all_w;

//�T���p
int way_count;
int wall_count;
int findX, findY;
int findX2, findY2;
int enumeration_count;//�S�񋓂����ʂ肩�L�^����
int permutation[100];//�S�񋓗p��0-1��𐶐����邽�߂̔z��

//����@�ɗp����e�ϐ�
int Ass;//����̐[���𓝊�
ASS data_ass[36][36];//����ɂ��Ă̏����L�^���Ă����z��
const int ASS_P = 1;//����̐e��\���萔

//queue�p�ϐ�
int IN, OUT;
const int QUEUE_MAX_SIZE = 1024;
Masu queue[1024];


/*:::::�֐��v���g�^�C�v�錾:::::*/

//���C������
void load_puzzle(char* filename);
void init();
void search();

//�����P�`�X���s���֐�
int unreachable_destroyer();//�����X
int search_for_master();//�����W
int slave_run();//�����V
int square_destroyer();//�����P
int master_grow();//�����R
int wall_grow();//�����Q
int interval_killer();//�����T
int diagonal_killer();//�����S
int fixed_domain_closer();//�����U

//�����P�O���s���֐�
int cand_destroyer();
int seek_for_start_point(int x, int y, int n, int id, int flag);//�����W�A�����X�ł��p����
int bfs(int x, int y, int n, int id);//�����W�A�����X�ł��p����
void enu(int N, int n, int need, int id);
void candidate_get(int x, int y);
int getS(int x, int y, int id);

//���菈����S���֐�
void ass_manager();
int is_strange_map();
int find_other_master(int x, int y, int id);
int is_closed_strange(int x, int y, int flag);
int is_exact_answer();
int united_count(int x, int y, int type);
void back_ago_ass();

//�ǂƗ̓y�̏������݂��ꊇ���čs���֐�
void wall_set(int x, int y);
void slave_set(int x, int y, int id);

//�����P�`�X�ŗ��p����T�u�֐�
void close(int x, int y);
int wall_grow_agent(int x, int y);
void number_grow_agent(int x, int y);
int slave_grow_agent(int x, int y);
void slave_grow_agent2(int x, int y);
int is_there_master(int x, int y);
void slave_call(int x, int y, int id);
void wall_check(int x, int y);

//Queue�֘A�֐�
void init_queue();
void add(Masu m);
Masu poll();
int qsize();

//�������y�у��[�e�B���e�B�֐�
void init_check_array(int x);
void init_check2_array(int x);
void init_bfs_array(int x);
void init_bfs_first_array();
void init_unreachable_array(int x);
void init_ass_array(int dep, int past_state, int past_dep, int is_parent);
int is_out_of_range(int x, int y);
int abs(int x);



/*:::::���C���֐�:::::*/

int main(int argc, char *args[]){

	char *filename;
	int walls;
	int i, j;
	clock_t start, end;

	if(argc != 2){
		fprintf(stderr, "Usage: %s puzzle_file\n", args[0]);
		exit(1);
	}

	filename = args[1];
	load_puzzle(filename);

	//�J�n�����v��
	start = clock();

	//������
	init();
	//�������܂�܂ŏ������J��Ԃ�
	while(1){
		if(state_of_program == CLEAR)break;
		search();
	}
	//�I�������v��
	end = clock();

	printf("==========================================\n");
	printf("###         solve the problem!!        ###\n");
	printf("==========================================\n\n");
  	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			if(map[i][j] == KABE)printf("*");
			else if(map[i][j] == SLAVE)printf("-");
			else printf("%d", map[i][j]);
		}
		printf("\n");
	}

	printf("\n%.3fsec needed\n", (double)(end-start)/CLOCKS_PER_SEC);
		


	return 0;
}

/*:::::�֐��S:::::*/

void load_puzzle(char* filename){
	int i, j, count=0;
	FILE* inputfile;
  
	if((inputfile = fopen(filename, "r")) == NULL){
		printf("file not exsit!!\n");
		exit(1);
	}

	fscanf(inputfile, "#%d %d\n", &X, &Y);
	for(i=0;i<Y;i++){
		fgets(c_map[i], 40, inputfile);
	}

	//int�^�ɂ��ă��C���}�b�v�Ɉڂ�
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			
			map[i][j] = c_map[j][i] - '0';
		}
	}

}

void init(){
	int i, j;
		
	//���݂̏󋵂�������
	state_of_program = SOLVING;
	//����̐[����������
	Ass = 0;
	//���݂̕ǂ̐���������
	now_w = 0;
	//�ǂ̑������v�Z�J�n
	all_w=X*Y;
	//���D��T���`�F�b�N�z���������
	init_bfs_array(-1);
	//����@�ɗp����z���������
	init_ass_array(0, SPACE, 0, 0);
		
	//�̎�}�b�v��-1�ŏ�����
	for(i=0;i<X;i++){
		for(j=0;j<Y;j++){
			master_map[i][j] = -1;
		}
	}
	//�̎�̐���������
	M = 0;
	//�z�� master[] �ɗ̎��id��t�^���Ċi�[����
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			
			if(map[i][j] != 0){
				master[M].n = map[i][j];
				master[M].x = i;
				master[M].y = j;
				master[M].id = M;
				master[M].is_closed = 0;
				master[M].s = 1;

				master_map[i][j] = M;
				M++;
				//�ǂ̑������Z�o
				all_w -= map[i][j];
			}
		}	
	}
		
}
void search() {

	int update = 0;
	if(state_of_program == CLEAR)return;
		
		
		
	update += fixed_domain_closer();
	update += diagonal_killer();
	update += interval_killer();
	update += wall_grow();
	update += master_grow();
	update += square_destroyer();
	update += slave_run();
		
	if(update==0)update += search_for_master();
	if(update==0)update +=cand_destroyer();
	if(update==0)update += unreachable_destroyer();
	if(update==0)ass_manager();
		
}

// method of �S�񋓂ɂ��e�̓y�Ɋւ���m��Ԃ�
int cand_destroyer(){
	int flag = 0;
	int cands;
	int i, j, k;
		
	//����`�F�b�N�z���������
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			check_cand[j][k].kabe = 0;
			check_cand[j][k].slave = 0;
			check_cand[j][k].condition = SPACE;
		}
	}
	//�S�񋓂̌��}�X�̐��̏���̂������l�̔�����
	if(Ass > 5)T = T-5;
	else if(Ass > 0)T = T - Ass;
		
	//�S�Ă̗̎�ɂ��đS�񋓏���������
	for(i=0;i<M;i++){
			
		if(master[i].is_closed || master[i].s == master[i].n)continue;
			
		init_check_array(0);
		init_bfs_array(-1);
		init_bfs_first_array();

		//check[][]�ɓ��B�\�}�X���L�^����(cands�̓}�X�̐�)
		cands = seek_for_start_point(master[i].x, master[i].y, master[i].n - master[i].s - 1, master[i].id,0);
			
		//�������l�𒴂������̂̓X�L�b�v
		if(cands > T)continue;
			
		//�񋓐���������
		enumeration_count = 0;
			
		//����̓y���܂߂Ă���(�m���̓y�͌v�Z���ɉ��Z�����)
		for(k=0;k<Y;k++){
			for(j=0;j<X;j++){
				if(map[j][k] == SLAVE && master_map[j][k] == master[i].id)check[j][k] = 1;
			}
		}
		//�S�񋓂����Acheck_cand[][]�ɕǂɂȂ蓾��񐔁A�̓y�ɂȂ肤��񐔂��L�^����
		enu(cands, 0, master[i].n-master[i].s, master[i].id);

		//�񋓐���0�ɂȂ�ꍇ�͔�΂�(�ʏ킠�肦�Ȃ����A����@��p����Əo�Ă���)
		if(enumeration_count == 0)continue;
			
		//�m�肵���ꏊ�͍̏W���A����`�F�b�N�z��͏�����
		for(k=0;k<Y;k++){
			for(j=0;j<X;j++){
				if(check_cand[j][k].kabe == enumeration_count){
					wall_set(j, k);
					flag = 1;
				}
				else if(check_cand[j][k].slave == enumeration_count){
					slave_set(j, k, master[i].id);
					flag = 1;
				}
				else if(check_cand[j][k].kabe + check_cand[j][k].slave == enumeration_count);

				//���������Ă���
				check_cand[j][k].kabe = 0;
				check_cand[j][k].slave = 0;

			}
		}
	}
		
	return flag;
}
//���D��T���œ��B�\�}�X�𒲂ׂ邽�߂̎n�_�T���B�̓y�ɗאڂ����󔒃}�X�S�Ă��n�_�ƂȂ�B�Ԃ�l�͓��B�\�ȃ}�X�̐�
int seek_for_start_point(int x, int y, int n, int id, int flag){
	int i;
	int res = 0;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] == KABE)return res;
		
	if(map[x][y] == SPACE){
		if(flag == 1)init_bfs_array(-1);
		res = bfs(x,y,n,id);
		return res;
	}
	check[x][y] = 1;
	for(i=0;i<4;i++)res += seek_for_start_point(x+directX[i], y+directY[i], n, id, flag);
	return res;
}
//���D��T���ɂ���āA�n�_(x,y)���狗��n�ȓ��ɂ���}�X��S�ċ��߂�(bfs_check[][]�ɂ��̃}�X�ɓ��B�ł���ő�c�苗��(����)���L�^����)�B�Ԃ�l�̓}�X�̐�
int bfs(int x, int y, int n, int id){
	int count = 0;
	int flag;
	int i;
	Masu m = {x, y, n};
	Masu tmp_m;
	init_queue();
	add(m);
	while(qsize() > 0){
		m = poll();
			
		//�͈͊O�A�ǁA�m��̓y���Ȃ���p
		if(is_out_of_range(m.x, m.y) || map[m.x][m.y] == KABE || check[m.x][m.y] == 1)continue;
		//���l�̗���̓y�Ȃ���p
		if(map[m.x][m.y] == SLAVE && master_map[m.x][m.y] != NORA && master_map[m.x][m.y] != id)continue;
		//�אڂ���}�X�ɑ��l�̗̓y������������p
		flag = 0;
		for(i=0;i<4;i++)if(!is_out_of_range(m.x+directX[i], m.y+directY[i]) && map[m.x+directX[i]][m.y+directY[i]] > 0){
			if(master_map[m.x+directX[i]][m.y+directY[i]] != NORA &&  master_map[m.x+directX[i]][m.y+directY[i]] != id)flag = 1;
		}
		if(flag == 1)continue;
				
		if(m.n > bfs_check[m.x][m.y]){
			if(bfs_check[m.x][m.y] == -1)count++;
			bfs_check[m.x][m.y] = m.n;
				
			//���B�\�ȗ̎傪�B��Ȃ�L�^���Ă���
			if(bfs_at_first_check[m.x][m.y] == MADA)bfs_at_first_check[m.x][m.y] = id;
			else if(bfs_at_first_check[m.x][m.y] >= 0 && bfs_at_first_check[m.x][m.y] != id)bfs_at_first_check[m.x][m.y] = DEL;
				
		}
		else continue;
			
		if(m.n == 0)continue;
		for(i=0;i<4;i++){
			tmp_m.x = m.x+directX[i];
			tmp_m.y = m.y+directY[i];
			tmp_m.n = m.n-1;
			add(tmp_m);
		}
	}
	return count;
}
/*�ċA�Ăяo���ɂ��A����N(=���ƂȂ�}�X�̐�)�̃r�b�g���int�^�z��̒l�Ƃ��ē���B
	N�̌��́A���ꂼ����W�ɂ���ď����t�����邩��A����ɂ���đS�񋓂���������B*/
void enu(int N, int n, int need, int id){
	int i, j, k;
	int count;
		
	if(N == n){
		count=0;
			
		//�r�b�g��Ɋ܂܂��1�̐�(=�̑������}�X�̐�)�𐔂���
		for(i=0;i<N;i++)if(permutation[i] == 1)count++;
			
		//�̑�����}�X�̐����̓y�ɕK�v�Ȑ��Ɠ����Ƃ��̂݁A�񋓂���
		if(count == need){
			for(k=0;k<Y;k++){
				for(j=0;j<X;j++){
					if(bfs_check[j][k] != -1)check[j][k] = permutation[--n];
					check_cand[j][k].condition = SPACE;
				}
			}
			//master�̈ʒu���琔���āA�����Ɨ̓y�̐���������Ă�����A�f�[�^�����
			init_check2_array(0);
			if(getS(master[id].x, master[id].y, id) == master[id].n){
				candidate_get(master[id].x, master[id].y);
				enumeration_count++;
			}
		}
		return;
	}
	permutation[n] = 0;
	enu(N,n+1,need,id);
	permutation[n] = 1;
	enu(N,n+1,need,id);
		
}
/*�񋓂��ꂽ��̊����̓y�ɂ��āA���̗̓y�A�y�ш͂��̕ǂ�����ꏊ�ɂ��āAcheck_cand[][]�Ƀf�[�^�����Z����
	�T���̃`�F�b�N�ɂ�check2[][]��p����B�܂��AgetS()�ɂ��A�����B�̃}�X��check[][]=1, ���֌W�̃}�X��check[][]=0�ƂȂ��Ă��邱�Ƃɒ��ӁB*/
void candidate_get(int x, int y){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 2 || map[x][y] == KABE || check_cand[x][y].condition != SPACE)return;
	if(check[x][y] == 0){
		if(map[x][y] == SPACE){
			check_cand[x][y].kabe++;
			check_cand[x][y].condition = KABE;
		}
		return;
	}
	if(map[x][y] == SPACE){
		check_cand[x][y].slave++;
		check_cand[x][y].condition = SLAVE;
	}
	check[x][y] = 2;
	for(i=0;i<4;i++)candidate_get(x+directX[i], y+directY[i]);
	check[x][y] = 1;
}
//�n�_����̗̓y�̖ʐς��ċA�I�ɋ��߂郁�\�b�h(�T���t���O�ɂ�check2[][]���g��)
int getS(int x, int y, int id){
	int res=0;
	int i;
		
	if(is_out_of_range(x,y) || check2[x][y] == 1)return res;
		
	/* [�m���̓y] or [�����̗���̓y]����Ȃ��āA���񋓂���Ă��Ȃ��Ȃ烊�^�[��
			[�m���̓y�ł���]��A�A[������(����)�̓y�ł���]��B�A[�񋓂���Ă��Ȃ�]��C�Ƃ����
			!(A || B) && C <=> !A && !B && C				*/
	if(!(map[x][y] == SLAVE && master_map[x][y] == NORA) && !(map[x][y] == SLAVE && master_map[x][y] == id) && check[x][y] != 1)return res;
		
	check2[x][y] = 1;
	for(i=0;i<4;i++)res += getS(x+directX[i], y+directY[i], id);
		
	return res+1;
}
	
// method of ����Ǘ�
void ass_manager(){
	//���肷��ꏊ�����������ǂ����̃t���O
	int i_could_ass = 0;
	int res;
	int j, k;
		
	if(Ass > 0){
		//����A�}�b�v�ɖ������Ȃ����`�F�b�N����
		res = is_strange_map();
		//���������������߂�
		if(res == 1){
			back_ago_ass();
			return;
		}

	}
	//����̃t�F�C�Y���J��グ��
	Ass++;
	//���肪�{����Ă��Ȃ��󔒃}�X��T���A���̃t�F�C�Y�̐e�Ƃ���
	for(k=0;(k<Y && i_could_ass==0);k++){
		for(j=0;(j<X && i_could_ass==0);j++){
			if(map[j][k] == SPACE && data_ass[j][k].dep == 0){
					
				data_ass[j][k].is_parent = 1;
				wall_set(j,k);
				i_could_ass = 1;
					
			}
		}
	}
		
	//���肷��ꏊ��������Ȃ�������
	if(i_could_ass == 0){
		//�����������ɂȂ��Ă��邩�`�F�b�N
		res = is_exact_answer();
		//�ʖڂȂ�
		if(res == 0){
			Ass--;
			back_ago_ass();
		}
		//�����������Ȃ�I��
		else{
			state_of_program = CLEAR;
			init_ass_array(0, SPACE, 0, 0);
		}
	}
		
}
//�}�b�v�ɖ��������邩�𔻒肷��(�S�Ă̖��������m����킯�ł͂Ȃ��B�����܂Ō������̂��߂̎}���ł���)
int is_strange_map(){
	int res;
	int k, j;

	//�ǂ̐������߂��Ă��Ȃ����̃`�F�b�N
	if(now_w > all_w)return 1;
		
		
	//�Ǘ������̕ǂ��Ȃ����`�F�b�N
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] == KABE && check[j][k] == 0){
				res = is_closed_strange(j, k, KABE);
				if(res > 0 && res != all_w)return 1;
			}
		}
	}
	//�̓y�̐�������Ȃ��̂ɕ����Ă��铇�͂Ȃ����`�F�b�N
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] > 0 && map[j][k] < 10 && check[j][k] == 0){
				res = is_closed_strange(j, k, MASTER);
				if(res > 0 && res != map[j][k])return 1;
			}
		}
	}
	//�̎傪���Ȃ��̂ɕ������͂Ȃ����`�F�b�N
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] == SLAVE && check[j][k] == 0 && is_closed_strange(j, k, SLAVE) > 0)return 1;
		}
	}
	//�̎傪��l���Ȃ����`�F�b�N
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] > 0 && map[j][k] < 10 && check[j][k] == 0 && find_other_master(j,k,master_map[j][k]) > 0)return 1;
		}
	}
	//�l�p�ǂ��ł��Ă��Ȃ����̃`�F�b�N
	for(k=0;k<Y-1;k++){
		for(j=0;j<X-1;j++){
			if(map[j][k] == KABE && map[j+1][k+1] == KABE && map[j+1][k] == KABE && map[j][k+1] == KABE)return 1;
		}
	}
		
	return 0;
}
//�̒n���ɕʂ̗̎傪���邩�ǂ����𔻒肷��
int find_other_master(int x, int y, int id){
	int res = 0;
	int i;
		
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] == KABE || map[x][y] == SPACE)return 0;
		
	//�Ⴄid�̓z��������
	if((map[x][y] > 0 && map[x][y] < 10) || (map[x][y] == SLAVE && master_map[x][y] != NORA)){
		if(master_map[x][y] != id)return 1;
	}
		
	check[x][y] = 1;
	for(i=0;i<4;i++)res += find_other_master(x+directX[i], y+directY[i], id);
	return res;
}
//���`�F�b�N(�Ǘ����Ȃ�(��ors�̓y)����Ԃ��A�Ǘ����ĂȂ������畉����Ԃ�)
int is_closed_strange(int x, int y, int flag){
	int res = 0;
	int i;
		
	if(is_out_of_range(x,y) || check[x][y] == 1)return 0;
	//�ǂɂ��Ă̒T��
	if(flag == KABE){
		if(map[x][y] > 0)return 0;
		if(map[x][y] == SPACE)return -10000;
	}
	//�̓y�ɂ��Ă̒T��
	else if(flag == MASTER){
		if(map[x][y] == KABE)return 0;
		if(map[x][y] == SPACE)return -10000;
	}
	//�̎�̂��Ȃ��̓y�ɂ��Ă̒T��
	else if(flag == SLAVE){
		if(map[x][y] == KABE)return 0;
		if(map[x][y] == SPACE)return -10000;
		if(map[x][y] > 0 && map[x][y] < 10)return -10000;
	}
		
		
	check[x][y] = 1;
	for(i=0;i<4;i++)res += is_closed_strange(x+directX[i], y+directY[i], flag);
	return res+1;
}
//�����������ɂȂ��Ă��邩���肷��֐�
int is_exact_answer(){
	int j, k;
	
	//�ǂ̐���������瑦���ɑʖ�
	if(now_w != all_w)return 0;
		
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){

			if(map[j][k] == KABE && check[j][k] == 0 && all_w != united_count(j,k,KABE))return 0;
				
		}
	}
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] > 0 && map[j][k] < 10 && map[j][k] != united_count(j,k,SLAVE))return 0;
		}
	}
	for(k=0;k<Y-1;k++){
		for(j=0;j<X-1;j++){
			if(map[j][k] == KABE && map[j+1][k+1] == KABE && map[j+1][k] == KABE && map[j][k+1] == KABE)return 0;
		}
	}
	return 1;
}
//�A�����Ă���(��or�̓y)�̐��𐔂���
int united_count(int x, int y, int type){
	int res = 0;
	int i;
		
	if(type == KABE){
		if(is_out_of_range(x,y) || map[x][y] != KABE || check[x][y] == 1)return res;
	}else if(type == SLAVE){
		if(is_out_of_range(x,y) || map[x][y] == KABE || check[x][y] == 1)return res;
	}
		
	check[x][y] = 1;
	for(i=0;i<4;i++){
		res += united_count(x+directX[i], y+directY[i], type);
	}
	return res+1;
}
//�����������߂�����
void back_ago_ass(){
	int i, j, k;
	
	if(Ass == 0)return;
	//����̃t�F�C�Y��߂��O�ɁA���݂̃t�F�C�Y�ŉ��肵�����̂����ɖ߂�(�e�́A�ǂł���Ɖ��肳�ꂽ���Ǝ��̂����Ȃ̂ŁA�t�F�C�Y-1�̒i�K�ŁA�󔒂ł���K�v������ƌ�����)
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			//������Ƃ��K�v������
			for(i=0;i<2;i++){
				if(data_ass[j][k].dep == Ass){
						
					//�ǂ������Ȃ�ǐ��J�E���g���A�̓y�������Ȃ�̓y�J�E���g���f�N�������g
					if(map[j][k] == KABE)now_w--;
					//�m���ȊO
					else if((map[j][k] > 0 && map[j][k] < 10) || (map[j][k] == SLAVE && master_map[j][k] != NORA)){
						if(master[master_map[j][k]].is_closed)master[master_map[j][k]].is_closed = 0;
						master[ master_map[j][k] ].s--;
					}

						
					if(data_ass[j][k].past_state == NORA){
						map[j][k] = SLAVE;
						master_map[j][k] = NORA;
					}else {
						map[j][k] = SPACE;
						master_map[j][k] = -1;
					}
						
					data_ass[j][k].dep = data_ass[j][k].past_dep;
					//data_ass[j][k].dep = 0;
					data_ass[j][k].past_state = SPACE;
					data_ass[j][k].past_dep = 0;
						
					//�e�Ȃ�A�ӔC������̂ŁA��O�Ńm���̒n�Ƃ���
					if(data_ass[j][k].is_parent == ASS_P){
						map[j][k] = SLAVE;
						master_map[j][k] = NORA;
						data_ass[j][k].is_parent = 0;
						data_ass[j][k].dep = Ass-1;
							
					}
				}
			}
		}
	}
	//����̃t�F�C�Y����߂�
	Ass--;
}

// method of ���B�s�\�Ԃ�
int unreachable_destroyer(){
	int flag = 0;
	int i, j, k;
		
	init_unreachable_array(0);
	for(i=0;i<M;i++){
			
		if(master[i].is_closed || master[i].s == master[i].n)continue;
			
		//���D��T���ɂ��A���B�\�ȃ}�X�����߁Aunreachable_check[][]�ɋL�^����
		init_bfs_array(-1);
		init_bfs_first_array();
		init_check_array(0);
		seek_for_start_point(master[i].x, master[i].y, master[i].n - master[i].s - 1, master[i].id,0);
		for(k=0;k<Y;k++){
			for(j=0;j<X;j++){
					if(bfs_check[j][k] != -1)unreachable_check[j][k] = 1;
			}
		}
			
	}
	//���B�s�\�ȃ}�X�͕ǂɂ���
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
				if(unreachable_check[j][k] == 0 && map[j][k] == SPACE){
					wall_set(j,k);
					flag = 1;
				}
		}
	}
		
	return flag;
}
// method of �m���̒n�̗̎�T��
int search_for_master(){
	int flag = 0;
	int i, j, k;
		
	init_bfs_array(-1);
	init_bfs_first_array();
	for(i=0;i<M;i++){
			
		if(master[i].is_closed || master[i].s == master[i].n)continue;
			
		init_check_array(0);
		seek_for_start_point(master[i].x, master[i].y, master[i].n - master[i].s - 1, master[i].id,1);
	}
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
				if(bfs_at_first_check[j][k] >= 0 && map[j][k] == SLAVE && master_map[j][k] == NORA){
					slave_set(j,k,bfs_at_first_check[j][k]);
					flag = 1;
				}
		}
		
	}
	return flag;
}
// methos of �̎�̂��Ȃ��̒n�L�΂�
int slave_run(){
	int flag = 0;
	int found_master;
	int i, j, s, t;
	init_check_array(0);
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			//�m���̒n�Ȃ�T������
			if(map[i][j] == SLAVE && master_map[i][j] == NORA && check[i][j] == 0){
					
				//�L�΂���}�X�̐�
				way_count = 0;
				//�L�΂���ꏊ���L�^���Ă���
				findX = -1;
				findY = -1;
				found_master = slave_grow_agent(i,j);

					
				//�傪����������A�F�ɒm�点��
				if(found_master != -1){
					slave_call(i, j, found_master);
				}
				//�ЂƂ����L�΂���ꏊ���Ȃ��Ȃ�A�����ɍs�������Ȃ�
				else if(way_count == 1){
						
					slave_set(findX, findY, NORA);
					flag = 1;
				}

			}
			//(�Ǘ�)�̒n�ɂ��Ă��T������
			else if(map[i][j] == SLAVE && master_map[i][j] != NORA && check[i][j] ==0){
					
				for(t=0;t<Y;t++){
					for(s=0;s<X;s++){
						check2[s][t] = 0;
					}
				}
				if(is_there_master(i,j) == 0){
					//�L�΂���}�X�̐�
					way_count = 0;
					//�L�΂���ꏊ���L�^���Ă���
					findX = -1;
					findY = -1;
					slave_grow_agent2(i,j);
						
					if(way_count == 1){
							
						slave_set(findX, findY, master_map[i][j]);
						flag = 1;

					}
				}
			}
				
		}	
	}
		
	return flag;
}
// method of �l�p�Ԃ�
int square_destroyer(){
	int flag = 0;
	int count;
	int i, j, k;
	for(j=0;j<Y-1;j++){
		for(i=0;i<X-1;i++){
			count=0;
			for(k=0;k<4;k++){
				if(map[i+direct2X[k]][j+direct2Y[k]] == KABE)count++;
			}
			if(count == 3){
				for(k=0;k<4;k++){
					if(map[i+direct2X[k]][j+direct2Y[k]] == SPACE){
						slave_set(i+direct2X[k], j+direct2Y[k] , NORA);
						flag = 1;
					}
				}
			}
		}
	}
		
	return flag;
}
// method of �����L�΂�
int master_grow(){
	int flag = 0;
	int i, j;
	init_check_array(0);
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			//�̎�̃}�X�Ȃ�T��������(�m�蕕�����Ă��疳�ʂȂ̂ŁA�r������)
			if(map[i][j] > 0 && map[i][j] < 10 && !master[master_map[i][j]].is_closed){
					
				//�L�΂���}�X�̐�
				way_count = 0;
				//�L�΂���ꏊ���L�^���Ă���
				findX = -1;
				findY = -1;
				findX2 = -1;
				findY2 = -1;
				number_grow_agent(i,j);
					
				//�ЂƂ����L�΂���ꏊ���Ȃ��āA���̓y������Ă��Ȃ��Ȃ�A��邵���Ȃ�
				if(way_count == 1 && master[master_map[i][j]].s < map[i][j]){
						
					slave_set(findX, findY, master_map[i][j]);
					flag = 1;
				}
				//�L�΂���ꏊ���Ίp�ɂ���2�}�X�����ŁA����ɑ���Ȃ��̓y����}�X�����Ȃ�A�Ԃ̃}�X�͕ǂƂȂ�
				else if(way_count == 2 && master[master_map[i][j]].s+1 == map[i][j] && abs(findX-findX2) == 1 && abs(findY-findY2) == 1){
					if(map[findX][findY2] == SPACE)wall_set(findX, findY2);
					if(map[findX2][findY] == SPACE)wall_set(findX2, findY);
				}
			}
		}	
			
	}
	return flag;
}
// method of �ǐL�΂�
int wall_grow(){
	int flag = 0;
	int i, j;
	init_check_array(0);
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			if(map[i][j] == KABE && check[i][j] == 0){
					
				//�L�΂���}�X�̐�
				way_count = 0;
				//�L�΂���ꏊ���L�^���Ă���
				findX = -1;
				findY = -1;
				wall_count = wall_grow_agent(i,j);

					
				if(way_count == 1 && wall_count != now_w){
					wall_set(findX, findY);
					wall_check(findX,findY);
					flag = 1;
				}
			}
		}	
	}
	return flag;
}
// method of �ԎE�� �̎�y�ї̓y�ɂ���
int interval_killer(){
	int flag = 0;
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			//�������ɂ���
			if(!is_out_of_range(i+2,j) && map[i][j] > 0 && map[i+2][j] > 0){
				if(master_map[i][j] != master_map[i+2][j] && master_map[i][j] != NORA && master_map[i+2][j] != NORA){
					if(map[i+1][j] == SPACE){
						wall_set(i+1, j);
						flag = 1;
					}
						
				}
			}
			//�c�����ɂ���
			if(!is_out_of_range(i,j+2) && map[i][j] > 0 && map[i][j+2] > 0){
				if(master_map[i][j] != master_map[i][j+2] && master_map[i][j] != NORA && master_map[i][j+2] != NORA){
					if(map[i][j+1] == SPACE){
						wall_set(i, j+1);
						flag = 1;
					}
				}
			}
		}
	}
	return flag;
}
// method of �i�i���E�� �̎�y�ї̓y�ɂ���
int diagonal_killer(){
	int flag = 0;
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			//�E���ɂ���
			if(!is_out_of_range(i+1,j+1) && map[i][j] > 0 && map[i+1][j+1] > 0){
				if(master_map[i][j] != master_map[i+1][j+1] && master_map[i][j] != NORA && master_map[i+1][j+1] != NORA){
					if(map[i+1][j] == SPACE){
						wall_set(i+1, j);
						flag = 1;
					}
					if(map[i][j+1] == SPACE){
						wall_set(i, j+1);
						flag = 1;
					}	
				}
			}
			//�����ɂ���
			if(!is_out_of_range(i-1,j+1) && map[i][j] > 0 && map[i-1][j+1] > 0){
				if(master_map[i][j] != master_map[i-1][j+1] && master_map[i][j] != NORA && master_map[i-1][j+1] != NORA){
					if(map[i-1][j] == SPACE){
						wall_set(i-1, j);
						flag = 1;
					}
					if(map[i][j+1] == SPACE){
						wall_set(i, j+1);
						flag = 1;
					}	
				}
			}
		}
	}
	return flag;
}
// method of �m��̓y��
int fixed_domain_closer(){
	int flag = 0;
	int i;
	for(i=0;i<M;i++){
		if(!master[i].is_closed && master[i].n == master[i].s){
			init_check_array(0);
			close(master[i].x, master[i].y);
			master[i].is_closed = 1;
			flag = 1;
		}
	}
	return flag;
}

//�ǂ��Z�b�g����
void wall_set(int x, int y){
	if(map[x][y] != 0)return;
		
	//������̃Z�b�g
	data_ass[x][y].past_dep = data_ass[x][y].dep;//�O�̐[��������������
	if(map[x][y] == SLAVE && master_map[x][y] == NORA)data_ass[x][y].past_state = NORA;//�O�̏�Ԃ�����������
	else data_ass[x][y].past_state = map[x][y];
	data_ass[x][y].dep = Ass;
		
	//��������
	map[x][y] = KABE;
	now_w++;

}
//�̒n���Z�b�g����
void slave_set(int x, int y, int id){
		
	//������̃Z�b�g
	data_ass[x][y].past_dep = data_ass[x][y].dep;//�O�̐[��������������
	if(map[x][y] == SLAVE && master_map[x][y] == NORA)data_ass[x][y].past_state = NORA;//�O�̏�Ԃ�����������
	else data_ass[x][y].past_state = map[x][y];
	data_ass[x][y].dep = Ass;
		
	//��������
	map[x][y] = SLAVE;
	master_map[x][y] = id;
	if(id >= 0)master[id].s++;

}

//�m��̓y�������ċA�I�Ɏ��s����
void close(int x, int y){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] == KABE)return;
	if(map[x][y] == SPACE){
		wall_set(x,y);
		return;
	}
	check[x][y] = 1;
	for(i=0;i<4;i++)close(x+directX[i], y+directY[i]);
		
}
//�ǐL�΂����ċA�I�Ɏ��s���� (�Ԃ�l�͕ǂ̑����ł���A�L�΂���I�����̐����J�E���g����)
int wall_grow_agent(int x, int y){
	int wall_count = 0;
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] > 0)return 0;
		
	if(map[x][y] == SPACE){
		way_count++;
		findX =x;
		findY =y;
		return 0;
	}
	//���̎��_�Ń��^�[�����Ă��Ȃ� => �ǂł���
	check[x][y] = 1;
	for(i=0;i<4;i++)wall_count += wall_grow_agent(x+directX[i], y+directY[i]);
		
	return wall_count+1;
}
//�����L�΂����ċA�I�Ɏ��s���� (�L�΂���I�����̐����J�E���g����)
void number_grow_agent(int x, int y){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] == KABE)return ;
		
	//�m���̓y�ɓ��������ꍇ�́A�L�΂���ӏ�������Ȃ��Ȃ�̂ŁAway_count��傫�����Ă���
	if(master_map[x][y] == NORA){
		way_count = 10;
		//return;
	}
		
	if(map[x][y] == SPACE){
		way_count++;
		if(way_count == 1){
			findX =x;
			findY =y;
		}else if(way_count == 2){
			findX2 =x;
			findY2 =y;
		}
		return;
	}
	//���̎��_�Ń��^�[�����Ă��Ȃ� => slave or master
	check[x][y] = 1;
	for(i=0;i<4;i++)number_grow_agent(x+directX[i], y+directY[i]);

}
//Slave Run���ċA�I�Ɏ��s����(�Ԃ�l�͌��������̎��id)
int slave_grow_agent(int x, int y){
	int res = -1;
	int tmp_res;
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] == KABE)return res;
		
	if(map[x][y] == SPACE){
		way_count++;
		findX =x;
		findY =y;
		return res;
	}
	if(map[x][y] > 0 && master_map[x][y] != NORA){
		way_count = -1;
		return master_map[x][y];
	}
	//���̎��_�Ń��^�[�����Ă��Ȃ� => �m���̒n
	check[x][y] = 1;
	for(i=0;i<4;i++){
		tmp_res = slave_grow_agent(x+directX[i], y+directY[i]);
		if(tmp_res != -1)res = tmp_res; 
	}
	return res;
}
//�̎�̂���Ǘ��̒n��L�΂��������ċA�I�Ɏ��s����
void slave_grow_agent2(int x, int y){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] == KABE)return;

		
	if(map[x][y] == SPACE){
		way_count++;
		findX =x;
		findY =y;
		return;
	}

	check[x][y] = 1;
	for(i=0;i<4;i++){
		slave_grow_agent2(x+directX[i], y+directY[i]);
	}
}
//���̓��ɗ̎�͂��邩�H(�[���ŏ��������ꂽcheck2[][]���g�p)
int is_there_master(int x, int y){
	int flag = 0;
	int i;
	if(is_out_of_range(x,y) || check2[x][y] == 1 || map[x][y] == KABE || map[x][y] == SPACE)return flag;

		
	if(map[x][y] > 0 && map[x][y] < 10){
		flag = 1;
	}

	check2[x][y] = 1;
	for(i=0;i<4;i++){
		flag += is_there_master(x+directX[i], y+directY[i]);
	}
	return flag;
}
//slave call���ċA�I�Ɏ��s����(check[][]��1�̂Ƃ��ɖ��T���Ƃ���) 
void slave_call(int x, int y, int id){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 0 || master_map[x][y] != NORA)return;
		
	//���̎��_�Ń��^�[�����Ă��Ȃ� => �m���̒n
	check[x][y] = 0;
	for(i=0;i<4;i++)slave_call(x+directX[i], y+directY[i], id);
	slave_set(x, y, id);
}
//�����̈ʒu����A�����Ă���ǂ�S�ă`�F�b�N�ςɂ���
void wall_check(int x, int y){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] != KABE)return;
	check[x][y] = 1;
	for(i=0;i<4;i++)wall_check(x+directX[i], y+directY[i]);
}


//Queue����������֐��S

//Queue�̏�����
void init_queue(){
	IN = 0;
	OUT = 0;
}
//�v�f��ǉ�����
void add(Masu m){
	queue[IN++] = m;
	if(IN == OUT){
		printf("queue over flow");
		exit(1);
	}
	if(IN == QUEUE_MAX_SIZE)IN = 0;
}
//�擪�v�f�����o���A�폜����
Masu poll(){
	Masu m = queue[OUT++];
	if(OUT == QUEUE_MAX_SIZE)OUT = 0;
	return m;
}
//�v�f����Ԃ�
int qsize(){
	if(IN >= OUT)return IN - OUT;
	else return IN - OUT + QUEUE_MAX_SIZE;
}

	
//�z�������������֐��S
void init_check_array(int x){
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){	
			check[i][j] = x;
		}	
	}
}
void init_check2_array(int x){
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			check2[i][j] = x;
		}	
	}
}
void init_bfs_array(int x){
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			bfs_check[i][j] = x;
		}	
	}
}
void init_bfs_first_array(){
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			bfs_at_first_check[i][j] = MADA;
		}	
	}
}
void init_unreachable_array(int x){
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			unreachable_check[i][j] = x;
		}	
	}
}
void init_ass_array(int dep, int past_state, int past_dep, int is_parent){
	int i, j;
	for(i=0;i<X;i++){
		for(j=0;j<Y;j++){
			data_ass[i][j].dep = dep;
			data_ass[i][j].past_state = past_state;
			data_ass[i][j].past_dep = past_dep;
			data_ass[i][j].is_parent = is_parent;

		}	
	}
}
	
//�����̍��W���͈͊O�Ȃ�^��Ԃ�
int is_out_of_range(int x, int y){
		return (x < 0 || x >= X || y < 0 || y >= Y);
}
//��Βl�����߂�
int abs(int x){
	if(x>=0)return x;
	else return -x;
}





