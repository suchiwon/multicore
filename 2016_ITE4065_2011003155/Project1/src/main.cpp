/*
@FILE:		/src/main.cpp
@DATE:		2016/10/02
@AUTHOR:	서치원(suchiwon@hanyang.ac.kr)
@BRIEF:		메인 함수
*/

#include "global.cpp"
#include "bfs.cpp"

int main(int argc, char* argv[]) {
    
    int num_line_scan;						//입력 파일에서 읽을 그래프 정보의 줄 수
    int idx, num, adj;						//그래프 정보 저장에 사용할 변수
    
    //집의 개수 읽기
    scanf("%d", &g_house_num);

    //집의 교차로 번호 읽기
    for (int i = 0; i < g_house_num; i++) {
        scanf("%d", &g_house_vertex_num[i]);
    }

    //교차로 갯수와 그래프 정보 저장 줄 수 읽기
    scanf("%d %d", &g_vertex_size, &num_line_scan);

    //global.h의 전역 변수 초기화
    Init();

    //그래프 정보 읽기
    for (int i = 0; i < num_line_scan; i++) {
        scanf("%d %d", &idx, &num);

        for (int i = 0; i < num; i++) {
            scanf("%d", &adj);
    	    //비가중치 비방향 그래프이므로 양쪽에 edge 생성
            g_graph[g_edge_size[idx]++][idx] = adj;
            g_graph[g_edge_size[adj]++][adj] = idx;
        }
    }

    //bfs.cpp의 스레드 생성 및 최대, 최소값 계산 함수
    MakeThreadsAndSolve();

    return 0;
}
