/*
 * configuration.h
 *
 *  Created on: 2016年9月8日
 *      Author: ranger.shi
 */

#include "configuration.h"

#include <memory>
#include "bignum.h"
#include "uint256.h"
#include "arith_uint256.h"
#include "util.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <vector>

using namespace std;

#include "main.h"
#include "uint256.h"

#include <stdint.h>
#include "syncdatadb.h"

#include <boost/assign/list_of.hpp> // for 'map_list_of()'







namespace Checkpoints
{
    typedef map<int, uint256> MapCheckpoints; // the first parameter is  nHeight;
    CCriticalSection cs_checkPoint;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double SIGCHECK_VERIFICATION_FACTOR = 5.0;

    struct CCheckpointData {
        MapCheckpoints *mapCheckpoints;
        int64_t nTimeLastCheckpoint;
        int64_t nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    bool fEnabled = true;

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0, uint256S("1f0d05a703a917511558f046529c48ad53b55c5b16c5d432fab8773a4b5ed4f1"));
    static const CCheckpointData data = {
        &mapCheckpoints,
        0,      // * UNIX timestamp of last checkpoint block
        0,      // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
        0       // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, uint256S("c28af610f0fb593e6194cef9195f154327577fc20b50018ccc822a7940d2b92d"));

    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        0,
        0,
        0
    };

    static MapCheckpoints mapCheckpointsRegtest =
        boost::assign::map_list_of
        ( 0, uint256S("708d5c14424395963cd11bb3f2ff791f584efbeb59fe5922f2131bfc879cd1f7"))
        ;
    static const CCheckpointData dataRegtest = {
        &mapCheckpointsRegtest,
        0,
        0,
        0
    };

    const CCheckpointData &Checkpoints() {
        if (SysCfg().NetworkID() == TEST_NET)
            return dataTestnet;
        else if (SysCfg().NetworkID() == MAIN_NET)
            return data;
        else
            return dataRegtest;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    { //nHeight 找不到或 高度和hash都能找到，则返回true
        if (!fEnabled)
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex, bool fSigchecks) {
        if (pindex==NULL)
            return 0.0;

        int64_t nNow = time(NULL);

        double fSigcheckVerificationFactor = fSigchecks ? SIGCHECK_VERIFICATION_FACTOR : 1.0;
        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkpoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {    // 获取mapCheckpoints 中保存最后一个checkpoint 的高度
        if (!fEnabled)
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!fEnabled)
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

	bool LoadCheckpoint() {
		LOCK(cs_checkPoint);
		SyncData::CSyncDataDb db;
		return db.LoadCheckPoint(*Checkpoints().mapCheckpoints);
	}

	bool GetCheckpointByHeight(const int nHeight, std::vector<int> &vCheckpoints) {
		LOCK(cs_checkPoint);
		MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
		std::map<int, uint256>::iterator iterMap = checkpoints.upper_bound(nHeight);
		while (iterMap != checkpoints.end()) {
			vCheckpoints.push_back(iterMap->first);
			++iterMap;
		}
		return !vCheckpoints.empty();
	}

	bool AddCheckpoint(int nHeight, uint256 hash) {
		LOCK(cs_checkPoint);
		MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
		checkpoints.insert(checkpoints.end(), make_pair(nHeight, hash));
		return true;
	}

	void GetCheckpointMap(std::map<int, uint256> &mapCheckpoints) {
		LOCK(cs_checkPoint);
		const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
		mapCheckpoints = checkpoints;
	}

}


//=========================================================================
//========以下是静态成员初始化的值=====================================================


 const G_CONFIG_TABLE &IniCfg(){
	static G_CONFIG_TABLE * psCfg =  NULL;
     if(psCfg == NULL)
     {
    	 psCfg  = new G_CONFIG_TABLE ();
     }
     assert(psCfg != NULL);
     return *psCfg;

}
 const uint256 G_CONFIG_TABLE::GetIntHash(NET_TYPE type) const
 {

	 switch (type) {
		case MAIN_NET: {
			return uint256S(hashGenesisBlock_mainNet);
		}
		case TEST_NET: {
			return uint256S(hashGenesisBlock_testNet);
		}
		case RETTEST_NET: {
			return uint256S(hashGenesisBlock_regTest);
		}
		default:
			assert(0);
		}
		return uint256S("");

 }
const string G_CONFIG_TABLE::GetCheckPointPkey(NET_TYPE type) const {
	switch (type) {
	case MAIN_NET: {
		return CheckPointPK_MainNet;
	}
	case TEST_NET: {
		return CheckPointPK_TestNet;
	}
//			case RETTEST_NET: {
//				return std::move(uint256S(std::move(hashGenesisBlock_regTest)));
//			}
	default:
		assert(0);
	}
	return "";
}

 const vector<string> G_CONFIG_TABLE::GetIntPubKey(NET_TYPE type) const {

	 switch (type) {
		case MAIN_NET: {
			return intPubKey_mainNet;
		}
		case TEST_NET: {
			return initPubKey_testNet;
		}
		case RETTEST_NET: {
			return initPubkey_regTest;
		}
		default:
			assert(0);
		}
		return vector<string>();
	}


 const uint256 G_CONFIG_TABLE::GetHashMerkleRoot() const{
	 return uint256S(HashMerkleRoot);
 }

	 vector<unsigned int> G_CONFIG_TABLE::GetSeedNodeIP() const
	{
		return pnSeed;
	}

	 unsigned char* G_CONFIG_TABLE::GetMagicNumber(NET_TYPE type) const{
		 switch (type) {
			case MAIN_NET: {
				return Message_mainNet;
			}
			case TEST_NET: {
				return Message_testNet;
			}
			case RETTEST_NET: {
				return Message_regTest;
			}
			default:
				assert(0);
			}
			return NULL;
	}


	 vector<unsigned char> G_CONFIG_TABLE::GetAddressPrefix(NET_TYPE type,Base58Type BaseType) const{

		 switch (type) {
			case MAIN_NET: {
				return AddrPrefix_mainNet[BaseType];
			}
			case TEST_NET: {
				return AddrPrefix_testNet[BaseType];
			}
//			case RETTEST_NET: {
//				return Message_regTest;
//			}
			default:
				assert(0);
			}
			return vector<unsigned char>();

	}


	 unsigned int G_CONFIG_TABLE::GetnDefaultPort(NET_TYPE type) const{
		 switch (type) {
			case MAIN_NET: {
				return nDefaultPort_mainNet;
			}
			case TEST_NET: {
				return nDefaultPort_testNet;
			}
			case RETTEST_NET: {
				return nDefaultPort_regTest;
			}
			default:
				assert(0);
			}
			return 0;
	}
	 unsigned int G_CONFIG_TABLE::GetnRPCPort(NET_TYPE type) const{
		 switch (type) {
			case MAIN_NET: {
				return nRPCPort_mainNet;
			}
			case TEST_NET: {
				return nRPCPort_testNet;
			}
//			case RETTEST_NET: {
//				return Message_regTest;
//			}
			default:
				assert(0);
			}
			return 0;
	}
	 unsigned int G_CONFIG_TABLE::GetStartTimeInit(NET_TYPE type) const{
		 switch (type) {
				case MAIN_NET: {
					return StartTime_mainNet;
				}
				case TEST_NET: {
					return StartTime_testNet;
				}
				case RETTEST_NET: {
					return StartTime_regTest;
				}
				default:
					assert(0);
				}
				return 0;
		}

	 unsigned int G_CONFIG_TABLE::GetHalvingInterval(NET_TYPE type) const{
		 switch (type) {
				case MAIN_NET: {
					return nSubsidyHalvingInterval_mainNet;
				}
//				case TEST_NET: {
//					return nSubsidyHalvingInterval_testNet;
//				}
				case RETTEST_NET: {
					return nSubsidyHalvingInterval_regNet;
				}
				default:
					assert(0);
				}
				return 0;
	}

	 uint64_t G_CONFIG_TABLE::GetCoinInitValue() const{
		return InitialCoin;
	}
	 uint64_t G_CONFIG_TABLE::GetBlockSubsidyCfg(int nHeight) const{

//		 	uint64_t nSubsidy = 8 * COIN;
//		 	uint64_t nFixedValue = 1 * COIN;
//		     int halvings = nHeight / SysCfg().GetSubsidyHalvingInterval();
//		     // Force block reward to fixed value when right shift is more than 3.
//		     if (halvings > 3) {
//		     	return nFixedValue;
//		     }else {
//		 	    // Subsidy is cut in half every 2,590,000 blocks which will occur approximately every 5 years.
//		 		nSubsidy >>= halvings;
//		     }
//		     return nSubsidy;
			 return DefaultFee * COIN;
	}

//=========================================================================
//========以下是静态成员初始化的值=====================================================
//=========================================================================

//名称
string G_CONFIG_TABLE::COIN_NAME = "StarShares";


	//公钥-主网络
 vector<string> G_CONFIG_TABLE::intPubKey_mainNet =
{
	"0358ea4aaa94f0ee458472b4c0cb4c314d69d8f8910d63a5c9adb7bb14d61805ca",
	"031bb7daf840ce317fb459f37591f85e4d9b48a96733b4806adae0ebfd657ca335"
};
	//公钥-测试网络
 vector<string> G_CONFIG_TABLE::initPubKey_testNet =
{
	"036d60c48da0cc2976acb1e98571131c9a8f15a0abb875c841994a342ed63100a6",
	"03b6c6e8f536948abe919160e1c7c1f588cce02e49edbce0816effb3293e800013"
};
	//公钥-局域网络
 vector<string> G_CONFIG_TABLE::initPubkey_regTest =
{
	"02adfa8c78d2bdf8e1dd72b8a5c091fe036f9760edd8f1463788e12b01d48443a7",
	"028eae1f9e8711ba85b7e2c458b33d021a429ce5ec11040d2f09c72fd6faf009cd",
	"02c2e678edc4579141ebf9b2a0be145d8418416f2e6a619c720b3794f073ef4d7d",
	"03e5911662c241b445b5897dc03ad89f0fb19baef71c64f7ec34381ef628b82227",
	"0354ba8fc2881b66985d5da114a2d6885b0098665af5f233d3b3d8bd87d17d179f",
};


 //公钥
 string G_CONFIG_TABLE::CheckPointPK_MainNet = "024951a85c87f898c0ec8d9a84ed42647afd15f468b26e4923457c0d2920299059";
 string G_CONFIG_TABLE::CheckPointPK_TestNet = "038af8f6323a09a661e3fee8492afefaa9187cf6e45e809033b88e84d3d360696f";

//创世块HASH
string G_CONFIG_TABLE::hashGenesisBlock_mainNet = "0x0b5df2d1c5db48cf637f9a3d29bc39b7221e20ddb51ef085b2c8f5c13cb31b33";
string G_CONFIG_TABLE::hashGenesisBlock_testNet = "0x354a796267aaf4c957391a6bed1fa8090040c4b31f962b3e9f457d98b84de513";
string G_CONFIG_TABLE::hashGenesisBlock_regTest = "0x16a683ec0c0be54117f38cb7105e8c9977865dfacc4a3f204e2e60b9f5561390";

 //梅根HASH
string G_CONFIG_TABLE::HashMerkleRoot = "0xe9c21ae4d92c385c3047ca1377117406afabfb6c0dad0236a19783da4796c045";



/*
 * 120.26.53.210 0xd2351a78 Dd!@#.890
 * 120.26.53.99 0x63351a78
 * 121.42.183.218 0xdab72a79
 * 115.28.66.106 0x6a421c73
 * 47.90.49.128 0x80315a2f
 * 120.25.58.91 0x5b3a1978
 */
//IP地址
 vector<unsigned int> G_CONFIG_TABLE::pnSeed = { 0xd2351a78, 0x63351a78,  0xdab72a79, 0x6a421c73, 0x80315a2f, 0x5b3a1978};


 //网络协议魔数
 unsigned char G_CONFIG_TABLE::Message_mainNet[MESSAGE_START_SIZE] = { 0xff, 0x50, 0x63, 0x63 };
 unsigned char G_CONFIG_TABLE::Message_testNet[MESSAGE_START_SIZE] = { 0xfd, 0x65, 0x7d, 0x7d };
 unsigned char G_CONFIG_TABLE::Message_regTest[MESSAGE_START_SIZE] = { 0xfe, 0x60, 0x63, 0x7d };

 //修改地址前缀
 vector<unsigned char> G_CONFIG_TABLE::AddrPrefix_mainNet[MAX_BASE58_TYPES] = { { 63 } ,{ 25 },{ 128 } ,{ 0x41,0x1C,0xCC,0x3F } ,{ 0x41,0x1C,0x3A,0x4A } ,{0}};
 vector<unsigned char> G_CONFIG_TABLE::AddrPrefix_testNet[MAX_BASE58_TYPES] = { { 125 } ,{ 87 } ,{ 239 } ,{ 0x7F,0x54,0x3F,0x4D } ,{ 0x7F,0x57,0x5B,0x2C },{0}  };

//网络端口
 unsigned int G_CONFIG_TABLE::nDefaultPort_mainNet = 7761;
 unsigned int G_CONFIG_TABLE::nDefaultPort_testNet = 17761;
 unsigned int G_CONFIG_TABLE::nDefaultPort_regTest = 17771;

 unsigned int G_CONFIG_TABLE::nRPCPort_mainNet = 7770;
 unsigned int G_CONFIG_TABLE::nRPCPort_testNet = 17770;

//修改时间
 unsigned int G_CONFIG_TABLE::StartTime_mainNet = 1473671908;
 unsigned int G_CONFIG_TABLE::StartTime_testNet = 1473671947;
 unsigned int G_CONFIG_TABLE::StartTime_regTest = 1473671972;

//半衰期
 unsigned int G_CONFIG_TABLE::nSubsidyHalvingInterval_mainNet=2590000;
 unsigned int G_CONFIG_TABLE::nSubsidyHalvingInterval_regNet=500;

//修改发币初始值
 uint64_t G_CONFIG_TABLE::InitialCoin = 1000000000;

//矿工费用
 uint64_t G_CONFIG_TABLE::DefaultFee = 30;

