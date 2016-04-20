#include<stdio.h>
#include<stdlib.h>
#include<time.h>

/*　プログラムの実行の仕方
 *　./report2 sample10x10.pzl
 *　の様に、実行ファイルの引数にパズルのファイル名を渡して実行する
 */


//領主の情報を格納する構造体(sは現在の面積、nは自身の数字、is_closedは閉鎖してあるかどうか)
typedef struct{ int id, n, x, y, s, is_closed; }Master;
//幅優先探索用にマスの座標及び最短到達値を格納する構造体
typedef struct{ int x, y, n; }Masu;
//全列挙探索用に列挙数の情報を格納する構造体(conditionは、最後に行われた列挙における状態(KABE or SPACE))
typedef struct{	int kabe, slave, condition; }Candidate;
//仮定探索用に仮定の深さ及び仮定前の状態を格納する構造体(depは深さ、past_stateは仮定前のマスの状態(SPACE or NORA)is_parentは仮定の親であるかどうか)
typedef struct{	int dep, past_state, past_dep, is_parent; }ASS;


/*:::::グローバル変数郡:::::*/

//全列挙の候補マスの数の上限のしきい値
int T = 15;

//全体の状態
int state_of_program;
//サイズ格納
int X, Y;

//定数
const int KABE = -1;
const int SPACE = 0;
const int SLAVE = 10;
const int NORA = -2;
const int MADA = -1;//到達点を調べる際に使う
const int DEL = -2;//到達点を調べる際に使う
const int MASTER = 2;//矛盾判定で使う
const int SOLVING = 0;//回答中
const int CLEAR = 1;//答えが求まった

//マップ格納 [X][Y]
char c_map[36][36];//ファイルからの受け取り用
int map[36][36];//メインマップ(0=SPACE, 1-9=MASTER, 10=SLAVE)
int master_map[36][36];//領主マップ(その領地の領主のidを記録しておく。ただし、領主未確定の領地(ノラ領地)は値NORAを取る。
int check[36][36];//チェック配列。
int check2[36][36];//第2チェック配列。chec[][]だけでは不足の際に使用
int bfs_check[36][36];//幅優先探索用。最大残り歩数を記録
int bfs_at_first_check[36][36];//幅優先探索を用いて到達点を調べる際、そのマスに到達した領主を記録しておく(まだ到達していない=MADA、二人以上到達した=DEL、一人だけ到達した=id)
int unreachable_check[36][36];//到達不可能なマスをチェックしておく
Candidate check_cand[36][36];//候補カウント用

//四方向配列
int directX[] = {0, 1, 0,-1};
int directY[] = {1, 0,-1, 0};
//右下四方向配列
int direct2X[] = {0, 1, 0, 1};
int direct2Y[] = {0, 0, 1, 1};

//領主リスト
int M;
Master master[200];

//壁の数
int now_w;
int all_w;

//探索用
int way_count;
int wall_count;
int findX, findY;
int findX2, findY2;
int enumeration_count;//全列挙が何通りか記録する
int permutation[100];//全列挙用に0-1列を生成するための配列

//仮定法に用いる各変数
int Ass;//仮定の深さを統括
ASS data_ass[36][36];//仮定についての情報を記録しておく配列
const int ASS_P = 1;//仮定の親を表す定数

//queue用変数
int IN, OUT;
const int QUEUE_MAX_SIZE = 1024;
Masu queue[1024];


/*:::::関数プロトタイプ宣言:::::*/

//メイン処理
void load_puzzle(char* filename);
void init();
void search();

//処理１～９を行う関数
int unreachable_destroyer();//処理９
int search_for_master();//処理８
int slave_run();//処理７
int square_destroyer();//処理１
int master_grow();//処理３
int wall_grow();//処理２
int interval_killer();//処理５
int diagonal_killer();//処理４
int fixed_domain_closer();//処理６

//処理１０を行う関数
int cand_destroyer();
int seek_for_start_point(int x, int y, int n, int id, int flag);//処理８、処理９でも用いる
int bfs(int x, int y, int n, int id);//処理８、処理９でも用いる
void enu(int N, int n, int need, int id);
void candidate_get(int x, int y);
int getS(int x, int y, int id);

//仮定処理を担う関数
void ass_manager();
int is_strange_map();
int find_other_master(int x, int y, int id);
int is_closed_strange(int x, int y, int flag);
int is_exact_answer();
int united_count(int x, int y, int type);
void back_ago_ass();

//壁と領土の書き込みを一括して行う関数
void wall_set(int x, int y);
void slave_set(int x, int y, int id);

//処理１～９で利用するサブ関数
void close(int x, int y);
int wall_grow_agent(int x, int y);
void number_grow_agent(int x, int y);
int slave_grow_agent(int x, int y);
void slave_grow_agent2(int x, int y);
int is_there_master(int x, int y);
void slave_call(int x, int y, int id);
void wall_check(int x, int y);

//Queue関連関数
void init_queue();
void add(Masu m);
Masu poll();
int qsize();

//初期化及びユーティリティ関数
void init_check_array(int x);
void init_check2_array(int x);
void init_bfs_array(int x);
void init_bfs_first_array();
void init_unreachable_array(int x);
void init_ass_array(int dep, int past_state, int past_dep, int is_parent);
int is_out_of_range(int x, int y);
int abs(int x);



/*:::::メイン関数:::::*/

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

	//開始時刻計測
	start = clock();

	//初期化
	init();
	//解が求まるまで処理を繰り返す
	while(1){
		if(state_of_program == CLEAR)break;
		search();
	}
	//終了時刻計測
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

/*:::::関数郡:::::*/

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

	//int型にしてメインマップに移す
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			
			map[i][j] = c_map[j][i] - '0';
		}
	}

}

void init(){
	int i, j;
		
	//現在の状況を初期化
	state_of_program = SOLVING;
	//仮定の深さを初期化
	Ass = 0;
	//現在の壁の数を初期化
	now_w = 0;
	//壁の総数を計算開始
	all_w=X*Y;
	//幅優先探索チェック配列を初期化
	init_bfs_array(-1);
	//仮定法に用いる配列を初期化
	init_ass_array(0, SPACE, 0, 0);
		
	//領主マップを-1で初期化
	for(i=0;i<X;i++){
		for(j=0;j<Y;j++){
			master_map[i][j] = -1;
		}
	}
	//領主の数を初期化
	M = 0;
	//配列 master[] に領主のidを付与して格納する
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
				//壁の総数を算出
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

// method of 全列挙による各領土に関する確定つぶし
int cand_destroyer(){
	int flag = 0;
	int cands;
	int i, j, k;
		
	//候補列チェック配列を初期化
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			check_cand[j][k].kabe = 0;
			check_cand[j][k].slave = 0;
			check_cand[j][k].condition = SPACE;
		}
	}
	//全列挙の候補マスの数の上限のしきい値の微調整
	if(Ass > 5)T = T-5;
	else if(Ass > 0)T = T - Ass;
		
	//全ての領主について全列挙処理をする
	for(i=0;i<M;i++){
			
		if(master[i].is_closed || master[i].s == master[i].n)continue;
			
		init_check_array(0);
		init_bfs_array(-1);
		init_bfs_first_array();

		//check[][]に到達可能マスを記録する(candsはマスの数)
		cands = seek_for_start_point(master[i].x, master[i].y, master[i].n - master[i].s - 1, master[i].id,0);
			
		//しきい値を超えたものはスキップ
		if(cands > T)continue;
			
		//列挙数を初期化
		enumeration_count = 0;
			
		//離れ領土も含めておく(ノラ領土は計算時に加算される)
		for(k=0;k<Y;k++){
			for(j=0;j<X;j++){
				if(map[j][k] == SLAVE && master_map[j][k] == master[i].id)check[j][k] = 1;
			}
		}
		//全列挙をし、check_cand[][]に壁になり得る回数、領土になりうる回数を記録する
		enu(cands, 0, master[i].n-master[i].s, master[i].id);

		//列挙数が0になる場合は飛ばす(通常ありえないが、仮定法を用いると出てくる)
		if(enumeration_count == 0)continue;
			
		//確定した場所は採集し、候補列チェック配列は初期化
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

				//初期化しておく
				check_cand[j][k].kabe = 0;
				check_cand[j][k].slave = 0;

			}
		}
	}
		
	return flag;
}
//幅優先探索で到達可能マスを調べるための始点探し。領土に隣接した空白マス全てが始点となる。返り値は到達可能なマスの数
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
//幅優先探索によって、始点(x,y)から距離n以内にあるマスを全て求める(bfs_check[][]にそのマスに到達できる最大残り距離(歩数)を記録する)。返り値はマスの数
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
			
		//範囲外、壁、確定領土内なら棄却
		if(is_out_of_range(m.x, m.y) || map[m.x][m.y] == KABE || check[m.x][m.y] == 1)continue;
		//他人の離れ領土なら棄却
		if(map[m.x][m.y] == SLAVE && master_map[m.x][m.y] != NORA && master_map[m.x][m.y] != id)continue;
		//隣接するマスに他人の領土があったら棄却
		flag = 0;
		for(i=0;i<4;i++)if(!is_out_of_range(m.x+directX[i], m.y+directY[i]) && map[m.x+directX[i]][m.y+directY[i]] > 0){
			if(master_map[m.x+directX[i]][m.y+directY[i]] != NORA &&  master_map[m.x+directX[i]][m.y+directY[i]] != id)flag = 1;
		}
		if(flag == 1)continue;
				
		if(m.n > bfs_check[m.x][m.y]){
			if(bfs_check[m.x][m.y] == -1)count++;
			bfs_check[m.x][m.y] = m.n;
				
			//到達可能な領主が唯一なら記録しておく
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
/*再帰呼び出しにより、長さN(=候補となるマスの数)のビット列をint型配列の値として得る。
	N個の候補は、それぞれ座標によって順序付けられるから、それによって全列挙を実現する。*/
void enu(int N, int n, int need, int id){
	int i, j, k;
	int count;
		
	if(N == n){
		count=0;
			
		//ビット列に含まれる1の数(=採択されるマスの数)を数える
		for(i=0;i<N;i++)if(permutation[i] == 1)count++;
			
		//採択するマスの数が領土に必要な数と同じときのみ、列挙する
		if(count == need){
			for(k=0;k<Y;k++){
				for(j=0;j<X;j++){
					if(bfs_check[j][k] != -1)check[j][k] = permutation[--n];
					check_cand[j][k].condition = SPACE;
				}
			}
			//masterの位置から数えて、ちゃんと領土の数がそろっていたら、データを取る
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
/*列挙された一つの完成領土について、その領土、及び囲いの壁がある場所について、check_cand[][]にデータを加算する
	探索のチェックにはcheck2[][]を用いる。また、getS()により、未到達のマスはcheck[][]=1, 無関係のマスはcheck[][]=0となっていることに注意。*/
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
//始点からの領土の面積を再帰的に求めるメソッド(探索フラグにはcheck2[][]を使う)
int getS(int x, int y, int id){
	int res=0;
	int i;
		
	if(is_out_of_range(x,y) || check2[x][y] == 1)return res;
		
	/* [ノラ領土] or [自分の離れ領土]じゃなくて、かつ列挙されていないならリターン
			[ノラ領土である]をA、[自分の(離れ)領土である]をB、[列挙されていない]をCとすると
			!(A || B) && C <=> !A && !B && C				*/
	if(!(map[x][y] == SLAVE && master_map[x][y] == NORA) && !(map[x][y] == SLAVE && master_map[x][y] == id) && check[x][y] != 1)return res;
		
	check2[x][y] = 1;
	for(i=0;i<4;i++)res += getS(x+directX[i], y+directY[i], id);
		
	return res+1;
}
	
// method of 仮定管理
void ass_manager(){
	//仮定する場所があったかどうかのフラグ
	int i_could_ass = 0;
	int res;
	int j, k;
		
	if(Ass > 0){
		//現状、マップに矛盾がないかチェックする
		res = is_strange_map();
		//矛盾があったら一個戻る
		if(res == 1){
			back_ago_ass();
			return;
		}

	}
	//仮定のフェイズを繰り上げる
	Ass++;
	//仮定が施されていない空白マスを探し、このフェイズの親とする
	for(k=0;(k<Y && i_could_ass==0);k++){
		for(j=0;(j<X && i_could_ass==0);j++){
			if(map[j][k] == SPACE && data_ass[j][k].dep == 0){
					
				data_ass[j][k].is_parent = 1;
				wall_set(j,k);
				i_could_ass = 1;
					
			}
		}
	}
		
	//仮定する場所が見つからなかったら
	if(i_could_ass == 0){
		//正しい答えになっているかチェック
		res = is_exact_answer();
		//駄目なら
		if(res == 0){
			Ass--;
			back_ago_ass();
		}
		//正しい答えなら終了
		else{
			state_of_program = CLEAR;
			init_ass_array(0, SPACE, 0, 0);
		}
	}
		
}
//マップに矛盾があるかを判定する(全ての矛盾を感知するわけではない。あくまで効率化のための枝狩りである)
int is_strange_map(){
	int res;
	int k, j;

	//壁の数が超過していないかのチェック
	if(now_w > all_w)return 1;
		
		
	//孤立封鎖の壁がないかチェック
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] == KABE && check[j][k] == 0){
				res = is_closed_strange(j, k, KABE);
				if(res > 0 && res != all_w)return 1;
			}
		}
	}
	//領土の数が合わないのに閉鎖している島はないかチェック
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] > 0 && map[j][k] < 10 && check[j][k] == 0){
				res = is_closed_strange(j, k, MASTER);
				if(res > 0 && res != map[j][k])return 1;
			}
		}
	}
	//領主がいないのに閉じた島はないかチェック
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] == SLAVE && check[j][k] == 0 && is_closed_strange(j, k, SLAVE) > 0)return 1;
		}
	}
	//領主が二人いないかチェック
	init_check_array(0);
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			if(map[j][k] > 0 && map[j][k] < 10 && check[j][k] == 0 && find_other_master(j,k,master_map[j][k]) > 0)return 1;
		}
	}
	//四角壁ができていないかのチェック
	for(k=0;k<Y-1;k++){
		for(j=0;j<X-1;j++){
			if(map[j][k] == KABE && map[j+1][k+1] == KABE && map[j+1][k] == KABE && map[j][k+1] == KABE)return 1;
		}
	}
		
	return 0;
}
//領地内に別の領主がいるかどうかを判定する
int find_other_master(int x, int y, int id){
	int res = 0;
	int i;
		
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] == KABE || map[x][y] == SPACE)return 0;
		
	//違うidの奴がいたら
	if((map[x][y] > 0 && map[x][y] < 10) || (map[x][y] == SLAVE && master_map[x][y] != NORA)){
		if(master_map[x][y] != id)return 1;
	}
		
	check[x][y] = 1;
	for(i=0;i<4;i++)res += find_other_master(x+directX[i], y+directY[i], id);
	return res;
}
//閉鎖チェック(孤立閉鎖なら(壁ors領土)数を返し、孤立してなかったら負数を返す)
int is_closed_strange(int x, int y, int flag){
	int res = 0;
	int i;
		
	if(is_out_of_range(x,y) || check[x][y] == 1)return 0;
	//壁についての探索
	if(flag == KABE){
		if(map[x][y] > 0)return 0;
		if(map[x][y] == SPACE)return -10000;
	}
	//領土についての探索
	else if(flag == MASTER){
		if(map[x][y] == KABE)return 0;
		if(map[x][y] == SPACE)return -10000;
	}
	//領主のいない領土についての探索
	else if(flag == SLAVE){
		if(map[x][y] == KABE)return 0;
		if(map[x][y] == SPACE)return -10000;
		if(map[x][y] > 0 && map[x][y] < 10)return -10000;
	}
		
		
	check[x][y] = 1;
	for(i=0;i<4;i++)res += is_closed_strange(x+directX[i], y+directY[i], flag);
	return res+1;
}
//正しい答えになっているか判定する関数
int is_exact_answer(){
	int j, k;
	
	//壁の数が違ったら即座に駄目
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
//連結している(壁or領土)の数を数える
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
//仮定を一つ巻き戻す処理
void back_ago_ass(){
	int i, j, k;
	
	if(Ass == 0)return;
	//仮定のフェイズを戻す前に、現在のフェイズで仮定したものを元に戻す(親は、壁であると仮定されたこと自体が問題なので、フェイズ-1の段階で、空白である必要があると言える)
	for(k=0;k<Y;k++){
		for(j=0;j<X;j++){
			//二回やっとく必要がある
			for(i=0;i<2;i++){
				if(data_ass[j][k].dep == Ass){
						
					//壁を消すなら壁数カウントを、領土を消すなら領土カウントをデクリメント
					if(map[j][k] == KABE)now_w--;
					//ノラ以外
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
						
					//親なら、責任があるので、一個前でノラ領地とする
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
	//仮定のフェイズを一つ戻す
	Ass--;
}

// method of 到達不可能つぶし
int unreachable_destroyer(){
	int flag = 0;
	int i, j, k;
		
	init_unreachable_array(0);
	for(i=0;i<M;i++){
			
		if(master[i].is_closed || master[i].s == master[i].n)continue;
			
		//幅優先探索により、到達可能なマスを求め、unreachable_check[][]に記録する
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
	//到達不可能なマスは壁にする
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
// method of ノラ領地の領主探し
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
// methos of 領主のいない領地伸ばし
int slave_run(){
	int flag = 0;
	int found_master;
	int i, j, s, t;
	init_check_array(0);
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			//ノラ領地なら探索する
			if(map[i][j] == SLAVE && master_map[i][j] == NORA && check[i][j] == 0){
					
				//伸ばせるマスの数
				way_count = 0;
				//伸ばせる場所を記録しておく
				findX = -1;
				findY = -1;
				found_master = slave_grow_agent(i,j);

					
				//主が見つかったら、皆に知らせる
				if(found_master != -1){
					slave_call(i, j, found_master);
				}
				//ひとつしか伸ばせる場所がないなら、そこに行くしかない
				else if(way_count == 1){
						
					slave_set(findX, findY, NORA);
					flag = 1;
				}

			}
			//(孤立)領地についても探索する
			else if(map[i][j] == SLAVE && master_map[i][j] != NORA && check[i][j] ==0){
					
				for(t=0;t<Y;t++){
					for(s=0;s<X;s++){
						check2[s][t] = 0;
					}
				}
				if(is_there_master(i,j) == 0){
					//伸ばせるマスの数
					way_count = 0;
					//伸ばせる場所を記録しておく
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
// method of 四角つぶし
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
// method of 数字伸ばし
int master_grow(){
	int flag = 0;
	int i, j;
	init_check_array(0);
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			//領主のマスなら探索をする(確定封鎖してたら無駄なので、排除する)
			if(map[i][j] > 0 && map[i][j] < 10 && !master[master_map[i][j]].is_closed){
					
				//伸ばせるマスの数
				way_count = 0;
				//伸ばせる場所を記録しておく
				findX = -1;
				findY = -1;
				findX2 = -1;
				findY2 = -1;
				number_grow_agent(i,j);
					
				//ひとつしか伸ばせる場所がなくて、かつ領土が足りていないなら、取るしかない
				if(way_count == 1 && master[master_map[i][j]].s < map[i][j]){
						
					slave_set(findX, findY, master_map[i][j]);
					flag = 1;
				}
				//伸ばせる場所が対角にある2マスだけで、さらに足りない領土が一マスだけなら、間のマスは壁となる
				else if(way_count == 2 && master[master_map[i][j]].s+1 == map[i][j] && abs(findX-findX2) == 1 && abs(findY-findY2) == 1){
					if(map[findX][findY2] == SPACE)wall_set(findX, findY2);
					if(map[findX2][findY] == SPACE)wall_set(findX2, findY);
				}
			}
		}	
			
	}
	return flag;
}
// method of 壁伸ばし
int wall_grow(){
	int flag = 0;
	int i, j;
	init_check_array(0);
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			if(map[i][j] == KABE && check[i][j] == 0){
					
				//伸ばせるマスの数
				way_count = 0;
				//伸ばせる場所を記録しておく
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
// method of 間殺し 領主及び領土について
int interval_killer(){
	int flag = 0;
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			//横方向について
			if(!is_out_of_range(i+2,j) && map[i][j] > 0 && map[i+2][j] > 0){
				if(master_map[i][j] != master_map[i+2][j] && master_map[i][j] != NORA && master_map[i+2][j] != NORA){
					if(map[i+1][j] == SPACE){
						wall_set(i+1, j);
						flag = 1;
					}
						
				}
			}
			//縦方向について
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
// method of ナナメ殺し 領主及び領土について
int diagonal_killer(){
	int flag = 0;
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){
			//右下について
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
			//左下について
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
// method of 確定領土閉鎖
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

//壁をセットする
void wall_set(int x, int y){
	if(map[x][y] != 0)return;
		
	//仮定情報のセット
	data_ass[x][y].past_dep = data_ass[x][y].dep;//前の深さを書き換える
	if(map[x][y] == SLAVE && master_map[x][y] == NORA)data_ass[x][y].past_state = NORA;//前の状態を書き換える
	else data_ass[x][y].past_state = map[x][y];
	data_ass[x][y].dep = Ass;
		
	//書き込み
	map[x][y] = KABE;
	now_w++;

}
//領地をセットする
void slave_set(int x, int y, int id){
		
	//仮定情報のセット
	data_ass[x][y].past_dep = data_ass[x][y].dep;//前の深さを書き換える
	if(map[x][y] == SLAVE && master_map[x][y] == NORA)data_ass[x][y].past_state = NORA;//前の状態を書き換える
	else data_ass[x][y].past_state = map[x][y];
	data_ass[x][y].dep = Ass;
		
	//書き込み
	map[x][y] = SLAVE;
	master_map[x][y] = id;
	if(id >= 0)master[id].s++;

}

//確定領土封鎖を再帰的に実行する
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
//壁伸ばしを再帰的に実行する (返り値は壁の総数であり、伸ばせる選択肢の数をカウントする)
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
	//この時点でリターンしていない => 壁である
	check[x][y] = 1;
	for(i=0;i<4;i++)wall_count += wall_grow_agent(x+directX[i], y+directY[i]);
		
	return wall_count+1;
}
//数字伸ばしを再帰的に実行する (伸ばせる選択肢の数をカウントする)
void number_grow_agent(int x, int y){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] == KABE)return ;
		
	//ノラ領土に当たった場合は、伸ばせる箇所が判らなくなるので、way_countを大きくしておく
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
	//この時点でリターンしていない => slave or master
	check[x][y] = 1;
	for(i=0;i<4;i++)number_grow_agent(x+directX[i], y+directY[i]);

}
//Slave Runを再帰的に実行する(返り値は見つかった領主のid)
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
	//この時点でリターンしていない => ノラ領地
	check[x][y] = 1;
	for(i=0;i<4;i++){
		tmp_res = slave_grow_agent(x+directX[i], y+directY[i]);
		if(tmp_res != -1)res = tmp_res; 
	}
	return res;
}
//領主のいる孤立領地を伸ばす処理を再帰的に実行する
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
//その島に領主はいるか？(ゼロで初期化されたcheck2[][]を使用)
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
//slave callを再帰的に実行する(check[][]が1のときに未探索とする) 
void slave_call(int x, int y, int id){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 0 || master_map[x][y] != NORA)return;
		
	//この時点でリターンしていない => ノラ領地
	check[x][y] = 0;
	for(i=0;i<4;i++)slave_call(x+directX[i], y+directY[i], id);
	slave_set(x, y, id);
}
//引数の位置から連続している壁を全てチェック済にする
void wall_check(int x, int y){
	int i;
	if(is_out_of_range(x,y) || check[x][y] == 1 || map[x][y] != KABE)return;
	check[x][y] = 1;
	for(i=0;i<4;i++)wall_check(x+directX[i], y+directY[i]);
}


//Queueを実装する関数郡

//Queueの初期化
void init_queue(){
	IN = 0;
	OUT = 0;
}
//要素を追加する
void add(Masu m){
	queue[IN++] = m;
	if(IN == OUT){
		printf("queue over flow");
		exit(1);
	}
	if(IN == QUEUE_MAX_SIZE)IN = 0;
}
//先頭要素を取り出し、削除する
Masu poll(){
	Masu m = queue[OUT++];
	if(OUT == QUEUE_MAX_SIZE)OUT = 0;
	return m;
}
//要素数を返す
int qsize(){
	if(IN >= OUT)return IN - OUT;
	else return IN - OUT + QUEUE_MAX_SIZE;
}

	
//配列を初期化する関数郡
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
	
//引数の座標が範囲外なら真を返す
int is_out_of_range(int x, int y){
		return (x < 0 || x >= X || y < 0 || y >= Y);
}
//絶対値を求める
int abs(int x){
	if(x>=0)return x;
	else return -x;
}





