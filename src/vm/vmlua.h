#ifndef VMLUA_H_
#define VMLUA_H_

#include <cstdio>

#include <vector>
#include <string>
#include <memory>



using namespace std;
class CVmRunEvn;




class CVmlua {
public:
	CVmlua(const vector<unsigned char> & vRom,const vector<unsigned char> &InputData);
	~CVmlua();
	tuple<uint64_t,string> run(uint64_t maxstep,CVmRunEvn *pVmScriptRun);

private:
	unsigned char m_ExRam[65536];  //��ŵ��Ǻ�Լ���׵�contact����
	unsigned char m_ExeFile[65536];//��ִ���ļ� IpboApp.lua

};


#endif
