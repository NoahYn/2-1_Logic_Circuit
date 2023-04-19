#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <chrono>

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

int main(void) {
	ifstream fin("input_minterm.txt");
	ofstream fout("result.txt");

	auto start = chrono :: steady_clock::now();
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
	
	vector<minTerm> next[b_len+1]; // Implicant Table의 다음 열	
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
		uniq.clear();

		if (brk) break;
		b_len--;
	}
	for (auto &k : PIs) cout << k << '\n';
	
	vector<vector<bool>> table; // Essential PI를 찾기 위한 표
	vector<bool> row;
	int c = PIs.size(); // 열 사이즈
	int r = trueMT.size(); // 행 사이즈
	for (int i = 0; i < c; i++) { 
		for (int j = 0; j < r; j++) {
			if (binMatch(PIs[i], trueMT[j])) row.push_back(true);
			else row.push_back(false);
		}
		table.push_back(row);
		row.clear();
	}

// TODO : 7 유일한 열이 없을 때

	vector<string> EPIs; // Essential PIs
	for (int i = 0; i < r; i++) { // 테이블 세로로 확인 
		int cnt = 0;
		int pos = -1;
		for (int j = 0; j < c; j++) {
			if (table[j][i] == true) { // 테이블 세로(열) 확인
				cnt++; pos = j;
			}
		}
		if (cnt == 1) { // 열에서 유일하다면 Essential PI
// TODO : 여기서 유일한 열이 있었는지 없었는지 확인.
			EPIs.push_back(PIs[pos]); // EPI로 추가
			for (int k = 0; k < r; k++) // 가로(행) 다른 PIs 삭제
				table[pos][k] = false;
		}
	}

	b_len = EPIs[0].length(); // 비트 길이 다시 구하기
	int Or = EPIs.size() * 2 + 2; // EPI의 개수가 OR의 input 개수(input 개수 * 2 => NOR, + inverter(2))
	int trn_num = Or;
	int Not = 0;	int And = 0;
	for (auto &k : EPIs) {
		fout << k << '\n';
		int cnt_dash = 0; // '-' 대시 하나마다 AND input 하나씩 감소
		int cnt_zero = 0; // 인버터를 위해 0이 있는 자리 계산
		for (int i = 0; i < b_len; i++) {
			if (k[i] == '-') cnt_dash++;
			if (k[i] == '0') cnt_zero += (1 << i); // 0의 위치 정보를 2진법에서 10진법으로 전환
		}
		And = (b_len - cnt_dash) * 2 + 2;
		trn_num += And;
		Not = Not | cnt_zero; // 0이 있는 위치들을 합집합연산(각 인덱스별로 0이 있는지 없는지)
		while (Not) {
			if (Not % 2 == 1) trn_num += 2; // 0 한개마다 인버터 하나씩 추가(트랜지스터 두개씩)
			Not >>= 1;
		}
	}
	fout << "\nCost(# of transistors): " << trn_num;

	fin.close();
	fout.close();

	auto end = chrono::steady_clock::now();
	cout << 
	chrono::duration_cast<chrono::seconds>(end-start).count() << "s " <<
	chrono::duration_cast<chrono::milliseconds>(end-start).count() << "ms " <<
	chrono::duration_cast<chrono::microseconds>(end-start).count() << "us " <<
	chrono::duration_cast<chrono::nanoseconds>(end-start).count() << "ns\n";
}