/*
 * configuration.h
 *
 *  Created on: 2016��9��8��
 *      Author: ranger.shi
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <memory>
#include "bignum.h"
#include "uint256.h"
#include "arith_uint256.h"
#include "util.h"
#include "chainparams.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <vector>
#include <map>

using namespace std;

class CBlockIndex;
class uint256;

/** Block-chain checkpoints are compiled-in sanity checks.
 * They are updated every release or three.
 */
namespace Checkpoints
{
    // Returns true if block passes checkpoint checks
    bool CheckBlock(int nHeight, const uint256& hash);

    // Return conservative estimate of total number of blocks, 0 if unknown
    int GetTotalBlocksEstimate();

    // Returns last CBlockIndex* in mapBlockIndex that is a checkpoint
    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex);

    double GuessVerificationProgress(CBlockIndex *pindex, bool fSigchecks = true);

    bool AddCheckpoint(int nHeight, uint256 hash);

    bool GetCheckpointByHeight(const int nHeight, std::vector<int> &vCheckpoints);

    bool LoadCheckpoint();

    void GetCheckpointMap(std::map<int, uint256> &checkpoints);

    extern bool fEnabled;
}


typedef enum  {
	emNODE_1,
	emNODE_2,
	MAX_NODE//!< MAX_NETWORK_TYPES
}NODE_NUMBER;



class G_CONFIG_TABLE
{

public:
	string GetCoinName()const
	{
		return /*std::move(COIN_NAME)*/COIN_NAME;
	}

	const vector<string> GetIntPubKey(NET_TYPE type) const;
	const uint256 GetIntHash(NET_TYPE type) const;
	const string GetCheckPointPkey(NET_TYPE type) const;
	 const uint256 GetHashMerkleRoot() const;
	 vector<unsigned int> GetSeedNodeIP() const;
	 unsigned char* GetMagicNumber(NET_TYPE type) const;
	 vector<unsigned char> GetAddressPrefix(NET_TYPE type,Base58Type BaseType) const;
	 unsigned int GetnDefaultPort(NET_TYPE type) const;
	 unsigned int GetnRPCPort(NET_TYPE type) const;
	 unsigned int GetStartTimeInit(NET_TYPE type) const;
	 unsigned int GetHalvingInterval(NET_TYPE type) const;
	 uint64_t GetCoinInitValue() const;
	 uint64_t GetBlockSubsidyCfg(int nHeight) const;

private:

	//���ƣ�
	static string COIN_NAME ;

	//��Կ
	static  vector<string> intPubKey_mainNet;
	static  vector<string> initPubKey_testNet;
	static  vector<string> initPubkey_regTest;

	//������HASH
	static string hashGenesisBlock_mainNet;
	static string hashGenesisBlock_testNet;
	static string hashGenesisBlock_regTest;


	//check point ��Կ
	static string CheckPointPK_MainNet;
	static string CheckPointPK_TestNet;


	//÷��HASH
	static string HashMerkleRoot;

	//IP��ַ
	static vector<unsigned int> pnSeed;


	//����Э��ħ��
	static unsigned char Message_mainNet[MESSAGE_START_SIZE];
	static unsigned char Message_testNet[MESSAGE_START_SIZE];
	static unsigned char Message_regTest[MESSAGE_START_SIZE];

	//�޸ĵ�ַǰ׺
	static  vector<unsigned char> AddrPrefix_mainNet[MAX_BASE58_TYPES];
	static  vector<unsigned char> AddrPrefix_testNet[MAX_BASE58_TYPES];


	//����˿�
	static unsigned int nDefaultPort_mainNet ;
	static unsigned int nDefaultPort_testNet ;
	static unsigned int nDefaultPort_regTest;

	static unsigned int nRPCPort_mainNet;
	static unsigned int nRPCPort_testNet ;

	//�޸�ʱ��
	static unsigned int StartTime_mainNet;
	static unsigned int StartTime_testNet;
	static unsigned int StartTime_regTest;

	//��˥��
	static unsigned int nSubsidyHalvingInterval_mainNet;
	static unsigned int nSubsidyHalvingInterval_regNet;


	//�޸ķ�������
	static uint64_t InitialCoin;

	//�󹤷���
	static uint64_t DefaultFee;


};

const G_CONFIG_TABLE &IniCfg();



#endif /* CONFIGURATION_H_ */
