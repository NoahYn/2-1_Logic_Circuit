#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

class minTerm {
public :
	char value;
	string binary;
	bool isCmbnd;

	minTerm() { isCmbnd = false; }
	minTerm(string binary) { this->binary = binary; isCmbnd = false; }
};

int bincmp(string &bin1, string &bin2) {
	int cnt = 0;
	int pos = 0;
	for (int i = 0; i < bin1.length(); i++) {
		if (bin1[i] != bin2[i]) { 
			cnt++; // 다른 비트 개수
			pos = i; // 위치 저장
		}
	}
	return (cnt == 1) ? pos : -1; // 다른 게 하나면 위치 반환, or -1 반환
}

bool binMatch(string &bin1, string &bin2) {
	for (int i = 0; i < bin1.length(); i++) {
		if (bin1[i] == '-' || bin2[i] == '-') continue; // '-'는 0과 1 둘다 포함, match
		if (bin1[i] != bin2[i]) return false; // 비트가 다르면 false
	}
	return true;
}

void BackTracking(vector<int> &table, set<string> &EPIs, vector<string> &trueMT, vector<string> &PIs, int idx) {

}


// TODO : 제출할 때 삭제
string file[10] = {"input_minterm.txt", "test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt", "test6.txt", "test7.txt", "test8.txt", "test9.txt"};

int main(int argc, char *argv[]) {
	ofstream fout("result.txt");
	ifstream fin;

for (int fi = 0; fi < 6; fi++) {
//	ifstream fin("input_minterm.txt");
	if (argc == 2) fin.open(argv[1]);
	else fin.open(file[fi]);	
	fout << "\n-------------------\n#" << fi << " test case \n";

	int b_len; // 비트 길이
	fin >> b_len; 
	vector<minTerm> MT[b_len+1]; // minterms
	vector<string> PIs; // Prime Implicants
	vector<string> trueMT; // true minterms

	minTerm init; int ones; // mt 벡터 초기화
	while (!fin.eof()) {
		fin >> init.value >> init.binary;
		if (init.value == 'm') trueMT.push_back(init.binary); 
		ones = count(init.binary.begin(), init.binary.end(), '1'); // 1 개수 세기
		MT[ones].push_back(init); // 개수별로 정리
	}
	
	set<string> uniq; // 합쳐진 값 중 중복 거르기 위한 set
	while (2021202033) {
		bool brk = 1; // 루프 탈출 플래그. 더 이상 합칠 게 없으면 1로 유지. 합쳐지면 0.
		for (int i = 0; i < b_len; i++) { 
			for (auto &k : MT[i]) {
				for (auto &k2 : MT[i+1]) { // 해밍디스턴스 1인 그룹
					int pos;
					if ((pos = bincmp(k.binary, k2.binary)) != -1) { // 비트가 하나만 다를 때 위치(인덱스) 반환, or -1
						brk = 0; // 루프 한번 더
						k.isCmbnd = true; k2.isCmbnd = true; // 사용 표시

						string combined(k.binary);
						combined[pos] = '-'; // 겹치는 부분 '-' 표시
						uniq.emplace(combined);
					}
				}
			}
		}
		for (int i = 0; i < b_len+1; i++) {
			for (auto &k : MT[i]) {
				if (k.isCmbnd == false) 
					PIs.push_back(k.binary);
			}
			MT[i].clear();
			for (auto &k : uniq)
				if (count(k.begin(), k.end(), '1') == i) MT[i].push_back(k); // 새로 만든 것들 다시 1개수 순으로 저장
		}
		for (int i = 0; i < b_len + 1; i++)
			for (auto &k : MT[i]) fout << i << ' ' << k.binary << '\n';
		fout << '\n';
		uniq.clear();

		if (brk) break;
		b_len--;
	}

//	for (auto &k : PIs) cout << k << '\n';
	
	set<string> EPIs; // Essential PIs
	vector<int> table; // Essential PI를 찾기 위한 표. int형 정수를 통해 비트로 표현.
	int col = 0;

	int flag = 1; // 유일한 PI를 가진 행이 하나도 없을 경우
	for (int i = 0; i < trueMT.size(); i++) { // 열을 먼저 확인하기 위함
		col = 0;
		int cnt = 0; // 열에 PI가 얼마나 있는지 count
		int pos = -1; // PI가 하나일 때 EPI로 사용할 수 있게 위치 저장
		for (int j = 0; j < PIs.size(); j++) {
			if (binMatch(trueMT[i], PIs[j])) {
				col |= (1 << j);
				cnt++;
				pos = j;
			}
		}
		if (cnt == 1) { // 열에 PI 하나 -> Essential PI
			flag = 0;
			EPIs.emplace(PIs[pos]); 
			trueMT.erase(trueMT.begin()+i--); // 해당 true minterm은 필요 없어짐.
			if (!table.empty()) {
				for (int t = 0; t < i; t++) {
					if (table[t] & (1 << pos)) {
						table[t] = 0; // 방금 선택한 PI가 포함하는 범위. 해당 열 삭제
						trueMT.erase(trueMT.begin()+t--); i--;
					}
				}
			}
		}
		else table.push_back(col);
	}
	if (flag) 
		BackTracking(table, EPIs, trueMT, PIs, 0);

	int trans_num = 0; // 트랜지스터 개수
	b_len = PIs[0].length(); // 비트 길이 다시 구하기

	int Or = EPIs.size(); // EPI의 개수가 OR의 input 개수
	if (Or > 1) // 인풋 개수는 항상 2보다 큼 
		trans_num += (Or << 1) + 2; // (NOR(input * 2) + inverter(2) => OR)
	
	int Not = 0;	int And = 0;
	for (auto &k : EPIs) {
		fout << k << '\n';
		int cnt_dash = 0; // '-' 대시 하나마다 AND input 하나씩 감소
		int cnt_zero = 0; // 인버터를 위해 0이 있는 자리 계산
		for (int i = 0; i < b_len; i++) {
			if (k[i] == '-') cnt_dash++;
			if (k[i] == '0') cnt_zero += (1 << i); // 0의 위치 정보를 2진법에서 10진법으로 전환
		}
		And = b_len - cnt_dash;
		if (And > 1) // 인풋 개수는 항상 2보다 큼
			trans_num += (And << 1) + 2;  // (NAND(input * 2) + inverter(2) => AND)
		
		Not |= cnt_zero; // 0이 있는 위치들을 합집합연산(각 인덱스별로 0이 있는지 없는지)
	}
	while (Not) {
		if (Not % 2 == 1) trans_num += 2; // 0 한개마다 인버터 하나씩 추가(트랜지스터 두개씩)
		Not >>= 1;
	}
	
	fout << "\nCost(# of transistors): " << trans_num;

	fin.close();
}
	fout.close();
}