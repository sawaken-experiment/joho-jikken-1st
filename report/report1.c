#include<stdio.h>
#include<stdlib.h>
#define KABE -1
#define SPACE 0

//マップ
char map[36][36];
int check[36][36];
int X, Y;
int AnsOfWalls;//完成形に含まれる壁の数
//探索用に、四方向を格納した配列を用意しておく
int directX[4] = {0,1,0,-1};
int directY[4] = {1,0,-1,0};
//2x2チェック用
int directX2[4][3] = {{0,1,1},{1,1,0},{0,-1,-1},{-1,-1,0}};
int directY2[4][3] = {{1,1,0},{0,-1,-1},{-1,-1,0},{0,1,1}};

//関数プロトタイプ宣言
void load_puzzle(char* filename);
void print_board();
int input(int* walls);
int isfinish(int walls);
int search(int i, int j, int type);
int is_out_of_range(int x, int y);
void init_check_array(int x);
int check2x2(int x, int y);

int main(int argc, char *args[]){

  char *filename;
  int walls = 0;

  if(argc != 2){
    fprintf(stderr, "Usage: %s puzzle_file\n", args[0]);
    exit(1);
  }

  filename = args[1];
  load_puzzle(filename);

  print_board();


  while(1){

    if(input(&walls) && isfinish(walls)){
      printf("==========================================\n");
      printf("###       You solve the problem!!      ###\n");
      printf("==========================================\n");
      print_board();
      break;
    }

    print_board();

  }


  return 0;

}

void load_puzzle(char* filename){

  int i, j, count=0;
  FILE* inputfile;
  
  if((inputfile = fopen(filename, "r")) == NULL){
    printf("file not exsit!!\n");
    exit(1);
  }

  fscanf(inputfile, "#%d %d\n", &X, &Y);
  for(i=0;i<Y;i++){
    fgets(map[i], 36, inputfile);
  }

  //壁の数をカウントしておく
  for(i=0;i<Y;i++){
    for(j=0;j<X;j++){
      count += map[i][j] - '0';
    }
  }
  printf("\n");
  //壁の数
  AnsOfWalls = X * Y - count;

  fclose(inputfile);

}
void print_board(){

  int i, j;
  char c;

  for(i=0;i<Y;i++){
    for(j=0;j<X;j++){
      if((c=map[i][j]) == '0')printf("-");
      else printf("%c", c);
    }
    printf("\n");
  }


}

int input(int* walls){

  int x, y;
  printf("Input wall position (x,y): ");

  if(scanf("%d,%d", &x, &y) != 2){
    scanf("%*s");
    printf("Illegal input\n");
    return 0;
  }

  //範囲外ならはじく
  if(is_out_of_range(x, y)){
    printf("[Against the Rule] input out the range.\n");
    return 0;
  }
  //壁なら空白に戻す
  else if(map[y][x] == '*'){
    map[y][x] = '0';
    (*walls)--;
    return 1;
  }
  //空白の時
  else if(map[y][x] == '0'){
	//壁が2x2以上の塊にならないか判定
	if(check2x2(x, y)){
		printf("[Against the Rule] you cannot put a block at a cell as 2x2.\n");
		return 0;
	}
    map[y][x] = '*';
    (*walls)++;
    return 1;

  }
  //それ以外なら数字のマス
  else{
    printf("[Against the Rule] you cannot put a block at a cell with a number.\n");
    return 0;
  }

}

//盤面が正答かどうかを調べる関数
int isfinish(int walls){
  int i, j;
  
  //壁の数が間違っていたらOUT
  if(walls != AnsOfWalls)return 0;
  
  //壁が全て繋がっているか調べる
  init_check_array(0);
  for(i=0;i<Y;i++){
    for(j=0;j<X;j++){
      if(map[i][j] == '*' && check[i][j] == 0){
		  if(search(i, j, KABE) != AnsOfWalls)return 0;
	  }
	}
  }
  //全ての島が正しい領土数を持っているか調べる
  init_check_array(0);
  for(i=0;i<Y;i++){
    for(j=0;j<X;j++){
      //数字を見る度に、その場所の空白の数が正しいか判定する
      if(map[i][j] > '0' && map[i][j] <= '9' && check[i][j] == 0){
		if(search(i, j, SPACE) != map[i][j]-'0')return 0;
		
	  }
	}
  }

  return 1;
}

int search(int i, int j, int type){
  int k, res = 0;
  
  if(is_out_of_range(j, i) || check[i][j] != 0)return res;

  //壁の探索か領土の探索かで場合わけ
  if(type == KABE){
	  if(map[i][j] != '*')return res;
  }else if(type == SPACE){
	  if(map[i][j] == '*')return res;
  }

  check[i][j] = 1;
  for(k=0;k<4;k++){
    res += search(i+directY[k], j+directX[k], type);
  }

  return res+1;
}



int check2x2(int x, int y){
	int i, j, flag, count;
	for(i=0;i<4;i++){

		//範囲外でないか調べる
		flag = 0;
		for(j=0;j<3;j++){
			if(is_out_of_range(x+directX2[i][j], y+directY2[i][j]))flag = 1;
		}
		if(flag == 1)continue;

		//3マスとも壁ならその時点で駄目
		count = 0;
		for(j=0;j<3;j++){
			if(map[y+directY2[i][j]][x+directX2[i][j]] == '*')count++;
		}
		if(count == 3)return 1;
	}
	return 0;

}

void init_check_array(int x){
	int i, j;
	for(j=0;j<Y;j++){
		for(i=0;i<X;i++){	
			check[i][j] = x;
		}	
	}
}
int is_out_of_range(int x, int y){
  return (x < 0 || x >= X || y < 0 || y >= Y);
}

