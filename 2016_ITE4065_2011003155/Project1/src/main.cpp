/*
@FILE:		/src/main.cpp
@DATE:		2016/10/02
@AUTHOR:	��ġ��(suchiwon@hanyang.ac.kr)
@BRIEF:		���� �Լ�
*/

#include "global.cpp"
#include "bfs.cpp"

int main(int argc, char* argv[]) {
    
    int num_line_scan;						//�Է� ���Ͽ��� ���� �׷��� ������ �� ��
    int idx, num, adj;						//�׷��� ���� ���忡 ����� ����
    
    //���� ���� �б�
    scanf("%d", &g_house_num);

    //���� ������ ��ȣ �б�
    for (int i = 0; i < g_house_num; i++) {
        scanf("%d", &g_house_vertex_num[i]);
    }

    //������ ������ �׷��� ���� ���� �� �� �б�
    scanf("%d %d", &g_vertex_size, &num_line_scan);

    //global.h�� ���� ���� �ʱ�ȭ
    Init();

    //�׷��� ���� �б�
    for (int i = 0; i < num_line_scan; i++) {
        scanf("%d %d", &idx, &num);

        for (int i = 0; i < num; i++) {
            scanf("%d", &adj);
    	    //����ġ ����� �׷����̹Ƿ� ���ʿ� edge ����
            g_graph[g_edge_size[idx]++][idx] = adj;
            g_graph[g_edge_size[adj]++][adj] = idx;
        }
    }

    //bfs.cpp�� ������ ���� �� �ִ�, �ּҰ� ��� �Լ�
    MakeThreadsAndSolve();

    return 0;
}
