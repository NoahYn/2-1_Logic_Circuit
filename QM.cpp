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
			cnt++; // �ٸ� ��Ʈ ����
			pos = i; // ��ġ ����
		}
	}
	return (cnt == 1) ? pos : -1; // �ٸ� �� �ϳ��� ��ġ ��ȯ, or -1 ��ȯ
}

bool binMatch(string& bin1, string& bin2) {
	for (int i = 0; i < bin1.length(); i++) {
		if (bin1[i] == '-' || bin2[i] == '-') continue; // '-'�� 0�� 1 �Ѵ� ����, match
		if (bin1[i] != bin2[i]) return false; // ��Ʈ�� �ٸ��� false
	}
	return true;
}

size_t getNumTrans(set<string>& EPIs, int b_len) {
	size_t trans_num = 0; // Ʈ�������� ����

	size_t Or = EPIs.size(); // EPI�� ������ OR�� input ����
	if (Or > 1) // ��ǲ ������ �׻� 2���� ŭ 
		trans_num += (Or << 1) + 2; // (NOR(input * 2) + inverter(2) => OR)

	size_t Not = 0;	int And = 0;
	for (auto& k : EPIs) {
		int cnt_dash = 0; // '-' ��� �ϳ����� AND input �ϳ��� ����
		size_t pos_zero = 0; // �ι��͸� ���� 0�� �ִ� �ڸ� ���
		for (int i = 0; i < b_len; i++) {
			if (k[i] == '-') cnt_dash++;
			if (k[i] == '0') pos_zero += (1 << i); // 0�� ��ġ ������ 2�������� 10�������� ��ȯ
		}
		And = b_len - cnt_dash;
		if (And > 1) // ��ǲ ������ �׻� 2���� ŭ
			trans_num += (And << 1) + 2;  // (NAND(input * 2) + inverter(2) => AND)

		Not |= pos_zero; // 0�� �ִ� ��ġ���� �����տ���(�� �ε������� 0�� �ִ��� ������)
	}
	while (Not) {
		if (Not % 2 == 1) trans_num += 2; // 0 �Ѱ����� �ι��� �ϳ��� �߰�(Ʈ�������� �ΰ���)
		Not >>= 1;
	}
	return trans_num;
}

int main(int argc, char* argv[]) {
	ios::sync_with_stdio(0);
	ofstream fout("result.txt");
	ifstream fin("input_minterm.txt");
	
	int b_len; // ��Ʈ ����
	fin >> b_len;
	vector<minTerm>* MT = new vector<minTerm>[b_len + 1]; // minterms
	vector<string> PIs; // Prime Implicants
	vector<string> trueMT; // true minterms

	minTerm init; int ones; // mt ���� �ʱ�ȭ
	while (!fin.eof()) {
		fin >> init.value >> init.binary;
		if (init.value == 'm') trueMT.push_back(init.binary);
		ones = count(init.binary.begin(), init.binary.end(), '1'); // 1 ���� ����
		MT[ones].push_back(init); // �������� ����
	}

	set<string> uniq; // ������ �� �� �ߺ� �Ÿ��� ���� set
	while (2021202033) {
		bool brk = 1; // ���� Ż�� �÷���. �� �̻� ��ĥ �� ������ 1�� ����. �������� 0.
		for (int i = 0; i < b_len; i++) {
			for (auto& k : MT[i]) {
				for (auto& k2 : MT[i + 1]) { // �عֵ��Ͻ� 1�� �׷�
					int pos;
					if ((pos = bincmp(k.binary, k2.binary)) != -1) { // ��Ʈ�� �ϳ��� �ٸ� �� ��ġ(�ε���) ��ȯ, or -1
						brk = 0; // ���� �ѹ� ��
						k.isCmbnd = true; k2.isCmbnd = true; // ��� ǥ��

						string combined(k.binary);
						combined[pos] = '-'; // ��ġ�� �κ� '-' ǥ��
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
				if (count(k.begin(), k.end(), '1') == i) MT[i].push_back(k); // ���� ���� �͵� �ٽ� 1���� ������ ����
		}
		uniq.clear();

		if (brk) break;
		b_len--;
	}

	set<string> EPIs; // Essential PIs
	vector<size_t> table; // Essential PI�� ã�� ���� ǥ. ������ ���� ��Ʈ�� ǥ��.(�޸�, ����)
	int col = 0;

	for (int i = 0; i < trueMT.size(); i++) { // ���� ���� Ȯ���ϱ� ����
		col = 0;
		int cnt = 0; // ���� PI�� �󸶳� �ִ��� count
		int pos = -1; // PI�� �ϳ��� �� EPI�� ����� �� �ְ� ��ġ ����
		for (int j = 0; j < PIs.size(); j++) {
			if (binMatch(trueMT[i], PIs[j])) {
				col |= (1 << j);
				cnt++;
				pos = j;
			}
		}
		if (cnt == 1) { // ���� PI �ϳ� -> Essential PI
			EPIs.emplace(PIs[pos]);
			trueMT.erase(trueMT.begin() + i--); // �ش� true minterm�� �ʿ� ������.
			if (!trueMT.empty()) {
				for (int t = 0; t < i; t++) {
					if (table[t] & (1 << pos)) {
						table.erase(table.begin() + t); // ��� ������ PI�� �����ϴ� ����. �ش� �� ����
						trueMT.erase(trueMT.begin() + t--); i--;
					}
				}
			}
		}
		else table.push_back(col);
	}

	while (!trueMT.empty()) { // EPI �� ��ã�� ���
		int maxMt = 0; // �� ��� minterm�� �ִ� ����
		int maxpos = -1; // minterm�� ���� ���� ���� ��ġ
		
		if (trueMT.size() < 2) {
			for (int i = 0; i < PIs.size(); i++) { // �ϳ��� ���� ���
				if (table[0] & (1 << i)) {
					EPIs.emplace(PIs[i]);
					break;
				}
			}
		}

		for (int i = 0; i < PIs.size(); i++) { // maxMT, maxpos ã��
			int cnt = 0;
			for (int j = 0; j < trueMT.size(); j++) {
				if (table[j] & (1 << i)) cnt++; // MT�� ������ ī��Ʈ
			}
			if (maxMt < cnt) { // minterm�� �� ���� ���� �߰����� ��
				maxMt = cnt;
				maxpos = i;
			}
		}
		EPIs.emplace(PIs[maxpos]); // ã�� �� EPI�� ����
		for (int i = 0; i < trueMT.size(); i++) { // �ش� PI�� Ŀ���ϴ� minterm�� ����
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