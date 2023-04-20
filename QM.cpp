#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

class minTerm {
public:
	char value;
	string binary;
	bool isCmbnd;

	minTerm() { isCmbnd = false; }
	minTerm(string binary) { this->binary = binary; isCmbnd = false; }
};

int bincmp(string& bin1, string& bin2) {
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

bool binMatch(string& bin1, string& bin2) {
	for (int i = 0; i < bin1.length(); i++) {
		if (bin1[i] == '-' || bin2[i] == '-') continue; // '-'는 0과 1 둘다 포함, match
		if (bin1[i] != bin2[i]) return false; // 비트가 다르면 false
	}
	return true;
}

size_t getNumTrans(set<string>& EPIs, int b_len) {
	size_t trans_num = 0; // 트랜지스터 개수

	size_t Or = EPIs.size(); // EPI의 개수가 OR의 input 개수
	if (Or > 1) // 인풋 개수는 항상 2보다 큼 
		trans_num += (Or << 1) + 2; // (NOR(input * 2) + inverter(2) => OR)

	size_t Not = 0;	int And = 0;
	for (auto& k : EPIs) {
		int cnt_dash = 0; // '-' 대시 하나마다 AND input 하나씩 감소
		size_t pos_zero = 0; // 인버터를 위해 0이 있는 자리 계산
		for (int i = 0; i < b_len; i++) {
			if (k[i] == '-') cnt_dash++;
			if (k[i] == '0') pos_zero += (1 << i); // 0의 위치 정보를 2진법에서 10진법으로 전환
		}
		And = b_len - cnt_dash;
		if (And > 1) // 인풋 개수는 항상 2보다 큼
			trans_num += (And << 1) + 2;  // (NAND(input * 2) + inverter(2) => AND)

		Not |= pos_zero; // 0이 있는 위치들을 합집합연산(각 인덱스별로 0이 있는지 없는지)
	}
	while (Not) {
		if (Not % 2 == 1) trans_num += 2; // 0 한개마다 인버터 하나씩 추가(트랜지스터 두개씩)
		Not >>= 1;
	}
	return trans_num;
}

int main(int argc, char* argv[]) {
	ios::sync_with_stdio(0);
	ofstream fout("result.txt");
	ifstream fin("input_minterm.txt");
	
	int b_len; // 비트 길이
	fin >> b_len;
	vector<minTerm>* MT = new vector<minTerm>[b_len + 1]; // minterms
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
			for (auto& k : MT[i]) {
				for (auto& k2 : MT[i + 1]) { // 해밍디스턴스 1인 그룹
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
		for (int i = 0; i < b_len + 1; i++) {
			for (auto& k : MT[i]) {
				if (k.isCmbnd == false)
					PIs.push_back(k.binary);
			}
			MT[i].clear();
			for (auto& k : uniq)
				if (count(k.begin(), k.end(), '1') == i) MT[i].push_back(k); // 새로 만든 것들 다시 1개수 순으로 저장
		}
		uniq.clear();

		if (brk) break;
		b_len--;
	}

	set<string> EPIs; // Essential PIs
	vector<size_t> table; // Essential PI를 찾기 위한 표. 정수를 통해 비트로 표현.(메모리, 성능)
	int col = 0;

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
			EPIs.emplace(PIs[pos]);
			trueMT.erase(trueMT.begin() + i--); // 해당 true minterm은 필요 없어짐.
			if (!trueMT.empty()) {
				for (int t = 0; t < i; t++) {
					if (table[t] & (1 << pos)) {
						table.erase(table.begin() + t); // 방금 선택한 PI가 포함하는 범위. 해당 열 삭제
						trueMT.erase(trueMT.begin() + t--); i--;
					}
				}
			}
		}
		else table.push_back(col);
	}

	while (!trueMT.empty()) { // EPI 다 못찾은 경우
		int maxMt = 0; // 한 행당 minterm의 최대 개수
		int maxpos = -1; // minterm이 가장 많은 행의 위치
		
		if (trueMT.size() < 2) {
			for (int i = 0; i < PIs.size(); i++) { // 하나만 남은 경우
				if (table[0] & (1 << i)) {
					EPIs.emplace(PIs[i]);
					break;
				}
			}
		}

		for (int i = 0; i < PIs.size(); i++) { // maxMT, maxpos 찾기
			int cnt = 0;
			for (int j = 0; j < trueMT.size(); j++) {
				if (table[j] & (1 << i)) cnt++; // MT가 있으면 카운트
			}
			if (maxMt < cnt) { // minterm이 더 많은 행을 발견했을 때
				maxMt = cnt;
				maxpos = i;
			}
		}
		EPIs.emplace(PIs[maxpos]); // 찾은 행 EPI로 저장
		for (int i = 0; i < trueMT.size(); i++) { // 해당 PI가 커버하는 minterm들 삭제
			if (table[i] & (1 << maxpos)) {
				table.erase(table.begin() + i);
				trueMT.erase(trueMT.begin() + i--);
			}
		}
	}

	for (auto& k : EPIs) {
		fout << k << '\n';
	}	fout << "\nCost(# of transistors): " << getNumTrans(EPIs, PIs[0].length());

	fin.close();
	fout.close();
}
`