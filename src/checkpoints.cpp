// Copyright (c) 2009-2012 The Woochain developers
// Copyright (c) 2011-2017 The Woochain developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "db.h"
#include "main.h"
#include "txdb.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;   // hardened checkpoints

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0, hashGenesisBlockOfficial )
        
	(91,   uint256("0x00000004e4f62a243c58587b846acd0a076b9607130cd714163c5e4d610d34a7"))
	(92,   uint256("0x000000087d75cc298537d9af4806ab7b299f7c64cc5bf3dc7d02fa08ed48c0ba"))
	(2887, uint256("0x00000049865c5c6d45a98055e5501cd1965a84cb1a3ce9432da03462722a364e"))
	(2888, uint256("0x00000015b9a7d1adfbcea6ee15f033189141f55cd2f79fee3712c470d2ec60d5"))
	(4748, uint256("0x00000068572dfb0661a6dff439964cee6123118d3681f6263ef4a9ac7ec1e502"))
	(4749, uint256("0x000000780346e810e8295785e48d81d66c619b833b501cd652bd3114db886951"))
	(6244, uint256("0x00000025767e4dddb42182113876eb27cba2443ad7684fbb1acf2de4eb13e084"))
	(6245, uint256("0x00000034a0cb7262166562ad5706b8405cb45039223ec361bcd4d085ee1eb3e9"))
	(9317, uint256("0x00000002dbdeff17ef912896e09609f066befa83c6cb5a0c99b5a0f0c75ceb01"))
	(9318, uint256("0x0000005146057c2d867d25fd6fc6c10a0d390605ad43b6b5a29b3741bda19c3e"))
	(11151, uint256("0x00000016a3b92dd7b785c5d16ea49fe5043fad98367f4a16cb436a4430a54124"))
	(11152, uint256("0x00000023004e304fc4b5f4c6dc2fd20b3707a5368bc74eea380c7829a0b42ce5"))	
	(13771, uint256("0x0000000cd28fac09ac5c6dae98000b0d41d206a6869b08bce2ed330c8886bdb0"))
	(13772, uint256("0x0000001161bac4bcdbe5aecc9771290cf4c6eb7fd5f03f88630a134f1b972498"))
	(13926, uint256("0x0000001366c2dc4fb8adfd44214537eb0989792ed67014dc0cb974973a7b4b5a"))
	(13927, uint256("0x0000000dd58b89882976eba51a351b1e4682425c93bc2890cfc3e332b39663d5"))

        //coingo.vip

        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1511962881, // * UNIX timestamp of last checkpoint block
        1331659,   // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        1000.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet = 
        boost::assign::map_list_of
        ( 0, hashGenesisBlockTestNet )
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1338180505,
        16341,
        300
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
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
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    uint256 GetLatestHardenedCheckpoint()
    {
        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
        return (checkpoints.rbegin()->second);
    }
}
