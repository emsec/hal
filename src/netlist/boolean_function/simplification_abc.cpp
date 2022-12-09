#include "hal_core/netlist/boolean_function/simplification.h"

#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/spirit/home/x3.hpp>
#include <mutex>
#include <string.h>

extern "C" {
////////////////////////////////////////////////////////////////////////////
/// # Developer Note
/// Since ABC is implemented in C (and we link towards the libabc.so within
/// the HAL core), we have to forward declare and sometimes re-implement
/// certain extern data types and (helper) functions we use from ABC [1].
/// Note that the code below is copied directy from ABC (and each data type
/// or function is equipped with a '/// Copied from $FILENAME' to indiciate
/// the original function implementation from ABC. Moreover note that we
/// applied a code reformatting of various data types / functions in order
/// to fit within the HAL code style.
///
/// [1] https://people.eecs.berkeley.edu/~alanmi/abc/
///
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Type Defintions
////////////////////////////////////////////////////////////////////////////

/// Copied from "src/base/abc/abc.h", see above for link to ABC's repository
typedef enum
{
    ABC_FUNC_NONE = 0,    // 0:  unknown
    ABC_FUNC_SOP,         // 1:  sum-of-products
    ABC_FUNC_BDD,         // 2:  binary decision diagrams
    ABC_FUNC_AIG,         // 3:  and-inverter graphs
    ABC_FUNC_MAP,         // 4:  standard cell library
    ABC_FUNC_BLIFMV,      // 5:  BLIF-MV node functions
    ABC_FUNC_BLACKBOX,    // 6:  black box about which nothing is known
    ABC_FUNC_OTHER        // 7:  unused
} Abc_NtkFunc_t;

/// Copied from "src/base/abc/abc.h", see above for link to ABC's repository
typedef enum
{
    ABC_NTK_NONE = 0,    // 0:  unknown
    ABC_NTK_NETLIST,     // 1:  network with PIs/POs, latches, nodes, and nets
    ABC_NTK_LOGIC,       // 2:  network with PIs/POs, latches, and nodes
    ABC_NTK_STRASH,      // 3:  structurally hashed AIG (two input AND gates with c-attributes on edges)
    ABC_NTK_OTHER        // 4:  unused
} Abc_NtkType_t;

/// Copied from "src/base/abc/abc.h", see above for link to ABC's repository
typedef enum
{
    ABC_OBJ_NONE = 0,    //  0:  unknown
    ABC_OBJ_CONST1,      //  1:  constant 1 node (AIG only)
    ABC_OBJ_PI,          //  2:  primary input terminal
    ABC_OBJ_PO,          //  3:  primary output terminal
    ABC_OBJ_BI,          //  4:  box input terminal
    ABC_OBJ_BO,          //  5:  box output terminal
    ABC_OBJ_NET,         //  6:  net
    ABC_OBJ_NODE,        //  7:  node
    ABC_OBJ_LATCH,       //  8:  latch
    ABC_OBJ_WHITEBOX,    //  9:  box with known contents
    ABC_OBJ_BLACKBOX,    // 10:  box with unknown contents
    ABC_OBJ_NUMBER       // 11:  unused
} Abc_ObjType_t;

/// Copied from "src/aig/hop/hop.h", see above for link to ABC's repository
typedef enum
{
    AIG_NONE,      // 0: non-existent object
    AIG_CONST1,    // 1: constant 1
    AIG_PI,        // 2: primary input
    AIG_PO,        // 3: primary output
    AIG_AND,       // 4: AND node
    AIG_EXOR,      // 5: EXOR node
    AIG_VOID       // 6: unused object
} Hop_Type_t;

/// Copied from "src/misc/vec/vecPtr.h", see above for link to ABC's repository
typedef struct Vec_Ptr_t_ Vec_Ptr_t;
struct Vec_Ptr_t_
{
    int nCap;
    int nSize;
    void** pArray;
};

/// Copied from "src/misc/vec/vecInt.h", see above for link to ABC's repository
typedef struct Vec_Int_t_ Vec_Int_t;
struct Vec_Int_t_
{
    int nCap;
    int nSize;
    int* pArray;
};

/// Copied from "src/misc/vec/vecVec.h", see above for link to ABC's repository
typedef struct Vec_Vec_t_ Vec_Vec_t;
struct Vec_Vec_t_
{
    int nCap;
    int nSize;
    void** pArray;
};

/// List of forward declarations of specific ABC data types required within
/// the data types that have to be known at compile-time.
typedef struct Nm_Man_t_ Nm_Man_t;
typedef struct Abc_Des_t_ Abc_Des_t;
typedef struct Mem_Fixed_t_ Mem_Fixed_t;
typedef struct Mem_Step_t_ Mem_Step_t;
typedef struct Abc_ManTime_t_ Abc_ManTime_t;
typedef struct Abc_Aig_t_ Abc_Aig_t;
typedef struct Abc_Cex_t_ Abc_Cex_t;
typedef struct st__table st__table;
typedef struct Gia_Man_t_ Gia_Man_t;
typedef struct Abc_Nam_t_ Abc_Nam_t;
typedef struct Dsd_Manager_t_ DdManager;
typedef void (*Abc_Frame_Callback_BmcFrameDone_Func)(int, int, int);
typedef struct Vec_Wec_t_ Vec_Wec_t;
typedef int64_t abctime;

/// Copied from "src/misc/vec/vecVec.h", see above for link to ABC's repository
typedef struct Hop_Obj_t_ Hop_Obj_t;
struct Hop_Obj_t_    // 6 words
{
    union
    {
        void* pData;    // misc
        int iData;      // misc
    };
    union
    {
        Hop_Obj_t* pNext;    // strashing table
        int PioNum;          // the number of PI/PO
    };
    Hop_Obj_t* pFanin0;         // fanin
    Hop_Obj_t* pFanin1;         // fanin
    unsigned int Type : 3;      // object type
    unsigned int fPhase : 1;    // value under 000...0 pattern
    unsigned int fMarkA : 1;    // multipurpose mask
    unsigned int fMarkB : 1;    // multipurpose mask
    unsigned int nRefs : 26;    // reference count (level)
    int Id;                     // unique ID of the node
};

/// Copied from "src/aig/hop/hop.h", see above for link to ABC's repository
typedef struct Hop_Man_t_ Hop_Man_t;
struct Hop_Man_t_
{
    // AIG nodes
    Vec_Ptr_t* vPis;       // the array of PIs
    Vec_Ptr_t* vPos;       // the array of POs
    Vec_Ptr_t* vObjs;      // the array of all nodes (optional)
    Hop_Obj_t* pConst1;    // the constant 1 node
    Hop_Obj_t Ghost;       // the ghost node
    // AIG node counters
    int nObjs[AIG_VOID];    // the number of objects by type
    int nCreated;           // the number of created objects
    int nDeleted;           // the number of deleted objects
    // stuctural hash table
    Hop_Obj_t** pTable;    // structural hash table
    int nTableSize;        // structural hash table size
    // various data members
    void* pData;       // the temporary data
    int nTravIds;      // the current traversal ID
    int fRefCount;     // enables reference counting
    int fCatchExor;    // enables EXOR nodes
    // memory management
    Vec_Ptr_t* vChunks;      // allocated memory pieces
    Vec_Ptr_t* vPages;       // memory pages used by nodes
    Hop_Obj_t* pListFree;    // the list of free nodes
    // timing statistics
    abctime time1;
    abctime time2;
};

/// Copied from "src/base/abc/abc.h", see above for link to ABC's repository
typedef struct Abc_Ntk_t_ Abc_Ntk_t;
struct Abc_Ntk_t_
{
    // general information
    Abc_NtkType_t ntkType;    // type of the network
    Abc_NtkFunc_t ntkFunc;    // functionality of the network
    char* pName;              // the network name
    char* pSpec;              // the name of the spec file if present
    Nm_Man_t* pManName;       // name manager (stores names of objects)
    // components of the network
    Vec_Ptr_t* vObjs;     // the array of all objects (net, nodes, latches, etc)
    Vec_Ptr_t* vPis;      // the array of primary inputs
    Vec_Ptr_t* vPos;      // the array of primary outputs
    Vec_Ptr_t* vCis;      // the array of combinational inputs  (PIs, latches)
    Vec_Ptr_t* vCos;      // the array of combinational outputs (POs, asserts, latches)
    Vec_Ptr_t* vPios;     // the array of PIOs
    Vec_Ptr_t* vBoxes;    // the array of boxes
    Vec_Ptr_t* vLtlProperties;
    // the number of living objects
    int nObjCounts[ABC_OBJ_NUMBER];    // the number of objects by type
    int nObjs;                         // the number of live objs
    int nConstrs;                      // the number of constraints
    int nBarBufs;                      // the number of barrier buffers
    int nBarBufs2;                     // the number of barrier buffers
    // the backup network and the step number
    Abc_Ntk_t* pNetBackup;    // the pointer to the previous backup network
    int iStep;                // the generation number for the given network
    // hierarchy
    Abc_Des_t* pDesign;     // design (hierarchical networks only)
    Abc_Ntk_t* pAltView;    // alternative structural view of the network
    int fHieVisited;        // flag to mark the visited network
    int fHiePath;           // flag to mark the network on the path
    int Id;                 // model ID
    double dTemp;           // temporary value
    // miscellaneous data members
    int nTravIds;               // the unique traversal IDs of nodes
    Vec_Int_t vTravIds;         // trav IDs of the objects
    Mem_Fixed_t* pMmObj;        // memory manager for objects
    Mem_Step_t* pMmStep;        // memory manager for arrays
    void* pManFunc;             // functionality manager (AIG manager, BDD manager, or memory manager for SOPs)
    Abc_ManTime_t* pManTime;    // the timing manager (for mapped networks) stores arrival/required times for all nodes
    void* pManCut;              // the cut manager (for AIGs) stores information about the cuts computed for the nodes
    float AndGateDelay;         // an average estimated delay of one AND gate
    int LevelMax;               // maximum number of levels
    Vec_Int_t* vLevelsR;        // level in the reverse topological order (for AIGs)
    Vec_Ptr_t* vSupps;          // CO support information
    int* pModel;                // counter-example (for miters)
    Abc_Cex_t* pSeqModel;       // counter-example (for sequential miters)
    Vec_Ptr_t* vSeqModelVec;    // vector of counter-examples (for sequential miters)
    Abc_Ntk_t* pExdc;           // the EXDC network (if given)
    void* pExcare;              // the EXDC network (if given)
    void* pData;                // misc
    Abc_Ntk_t* pCopy;           // copy of this network
    void* pBSMan;               // application manager
    void* pSCLib;               // SC library
    Vec_Int_t* vGates;          // SC library gates
    Vec_Int_t* vPhases;         // fanins phases in the mapped netlist
    char* pWLoadUsed;           // wire load model used
    float* pLutTimes;           // arrivals/requireds/slacks using LUT-delay model
    Vec_Ptr_t* vOnehots;        // names of one-hot-encoded registers
    Vec_Int_t* vObjPerm;        // permutation saved
    Vec_Int_t* vTopo;
    Vec_Ptr_t* vAttrs;      // managers of various node attributes (node functionality, global BDDs, etc)
    Vec_Int_t* vNameIds;    // name IDs
    Vec_Int_t* vFins;       // obj/type info
};

/// Copied from "src/base/abc/abc.h", see above for link to ABC's repository
typedef struct Abc_Obj_t_ Abc_Obj_t;
struct Abc_Obj_t_    // 48/72 bytes (32-bits/64-bits)
{
    Abc_Ntk_t* pNtk;          // the host network
    Abc_Obj_t* pNext;         // the next pointer in the hash table
    int Id;                   // the object ID
    unsigned Type : 4;        // the object type
    unsigned fMarkA : 1;      // the multipurpose mark
    unsigned fMarkB : 1;      // the multipurpose mark
    unsigned fMarkC : 1;      // the multipurpose mark
    unsigned fPhase : 1;      // the flag to mark the phase of equivalent node
    unsigned fExor : 1;       // marks AIG node that is a root of EXOR
    unsigned fPersist : 1;    // marks the persistant AIG node
    unsigned fCompl0 : 1;     // complemented attribute of the first fanin in the AIG
    unsigned fCompl1 : 1;     // complemented attribute of the second fanin in the AIG
    unsigned Level : 20;      // the level of the node
    Vec_Int_t vFanins;        // the array of fanins
    Vec_Int_t vFanouts;       // the array of fanouts
    union
    {
        void* pData;    // the network specific data
        int iData;
    };    // (SOP, BDD, gate, equiv class, etc)
    union
    {
        void* pTemp;         // temporary store for user's data
        Abc_Obj_t* pCopy;    // the copy of this object
        int iTemp;
        float dTemp;
    };
};

/// Copied from "src/base/main/mainInt.h", see above for link to ABC's repository
typedef struct Abc_Frame_t_ Abc_Frame_t;
struct Abc_Frame_t_
{
    // general info
    char* sVersion;    // the name of the current version
    char* sBinary;     // the name of the binary running
    // commands, aliases, etc
    st__table* tCommands;    // the command table
    st__table* tAliases;     // the alias table
    st__table* tFlags;       // the flag table
    Vec_Ptr_t* aHistory;     // the command history
    // the functionality
    Abc_Ntk_t* pNtkCur;          // the current network
    Abc_Ntk_t* pNtkBestDelay;    // the current network
    Abc_Ntk_t* pNtkBestArea;     // the current network
    Abc_Ntk_t* pNtkBackup;       // the current network
    int nSteps;                  // the counter of different network processed
    int fSource;                 // marks the source mode
    int fAutoexac;               // marks the autoexec mode
    int fBatchMode;              // batch mode flag
    int fBridgeMode;             // bridge mode flag
    // save/load
    Abc_Ntk_t* pNtkBest;    // the current network
    float nBestNtkArea;     // best area
    float nBestNtkDelay;    // best delay
    int nBestNtkNodes;      // best nodes
    int nBestNtkLevels;     // best levels

    // output streams
    FILE* Out;
    FILE* Err;
    FILE* Hst;
    // used for runtime measurement
    double TimeCommand;    // the runtime of the last command
    double TimeTotal;      // the total runtime of all commands
    // temporary storage for structural choices
    Vec_Ptr_t* vStore;    // networks to be used by choice
    // decomposition package
    void* pManDec;     // decomposition manager
    void* pManDsd;     // decomposition manager
    void* pManDsd2;    // decomposition manager
    // libraries for mapping
    void* pLibLut;      // the current LUT library
    void* pLibBox;      // the current box library
    void* pLibGen;      // the current genlib
    void* pLibGen2;     // the current genlib
    void* pLibSuper;    // the current supergate library
    void* pLibScl;      // the current Liberty library
    void* pAbcCon;      // constraint manager
    // timing constraints
    char* pDrivingCell;    // name of the driving cell
    float MaxLoad;         // maximum output load
    // inductive don't-cares
    Vec_Int_t* vIndFlops;
    int nIndFrames;

    // new code
    Gia_Man_t* pGia;                     // alternative current network as a light-weight AIG
    Gia_Man_t* pGia2;                    // copy of the above
    Gia_Man_t* pGiaBest;                 // copy of the above
    Gia_Man_t* pGiaBest2;                // copy of the above
    Gia_Man_t* pGiaSaved;                // copy of the above
    int nBestLuts;                       // best LUT count
    int nBestEdges;                      // best edge count
    int nBestLevels;                     // best level count
    int nBestLuts2;                      // best LUT count
    int nBestEdges2;                     // best edge count
    int nBestLevels2;                    // best level count
    Abc_Cex_t* pCex;                     // a counter-example to fail the current network
    Abc_Cex_t* pCex2;                    // copy of the above
    Vec_Ptr_t* vCexVec;                  // a vector of counter-examples if more than one PO fails
    Vec_Ptr_t* vPoEquivs;                // equivalence classes of isomorphic primary outputs
    Vec_Int_t* vStatuses;                // problem status for each output
    Vec_Int_t* vAbcObjIds;               // object IDs
    int Status;                          // the status of verification problem (proved=1, disproved=0, undecided=-1)
    int nFrames;                         // the number of time frames completed by BMC
    Vec_Ptr_t* vPlugInComBinPairs;       // pairs of command and its binary name
    Vec_Ptr_t* vLTLProperties_global;    // related to LTL
    Vec_Ptr_t* vSignalNames;             // temporary storage for signal names
    char* pSpecName;
    void* pSave1;
    void* pSave2;
    void* pSave3;
    void* pSave4;
    void* pAbc85Ntl;
    void* pAbc85Ntl2;
    void* pAbc85Best;
    void* pAbc85Delay;
    void* pAbcWlc;
    Vec_Int_t* pAbcWlcInv;
    void* pAbcBac;
    void* pAbcCba;
    void* pAbcPla;
    Abc_Nam_t* pJsonStrs;
    Vec_Wec_t* vJsonObjs;
#ifdef ABC_USE_CUDD
    DdManager* dd;    // temporary BDD package
#endif
    Gia_Man_t* pGiaMiniAig;
    Gia_Man_t* pGiaMiniLut;
    Vec_Int_t* vCopyMiniAig;
    Vec_Int_t* vCopyMiniLut;
    int* pArray;
    int* pBoxes;
    void* pNdr;
    int* pNdrArray;

    Abc_Frame_Callback_BmcFrameDone_Func pFuncOnFrameDone;
};

////////////////////////////////////////////////////////////////////////////
// Function Forwared Declarations for ABC interface
////////////////////////////////////////////////////////////////////////////

void Abc_Start();
void Abc_Stop();

Abc_Frame_t* Abc_FrameGetGlobalFrame();

Abc_Ntk_t* Abc_NtkAlloc(Abc_NtkType_t, Abc_NtkFunc_t, int);
void Abc_NtkDelete(Abc_Ntk_t*);

Abc_Obj_t* Abc_NtkCreateObj(Abc_Ntk_t*, Abc_ObjType_t);

char* Abc_ObjAssignName(Abc_Obj_t*, char*, char*);

Abc_Obj_t* Abc_AigConst1(Abc_Ntk_t*);
Abc_Obj_t* Abc_AigAnd(Abc_Aig_t*, Abc_Obj_t*, Abc_Obj_t*);
Abc_Obj_t* Abc_AigOr(Abc_Aig_t*, Abc_Obj_t*, Abc_Obj_t*);
Abc_Obj_t* Abc_AigXor(Abc_Aig_t*, Abc_Obj_t*, Abc_Obj_t*);

void Abc_ObjAddFanin(Abc_Obj_t*, Abc_Obj_t*);
int Abc_AigCleanup(Abc_Aig_t*);

int Abc_NtkCheck(Abc_Ntk_t*);

int Cmd_CommandExecute(Abc_Frame_t*, const char*);
void Abc_FrameReplaceCurrentNetwork(Abc_Frame_t*, Abc_Ntk_t*);
void Abc_FrameClearVerifStatus(Abc_Frame_t*);

Abc_Ntk_t* Abc_NtkToNetlist(Abc_Ntk_t*);
int Abc_NtkToAig(Abc_Ntk_t*);
Hop_Obj_t* Hop_IthVar(Hop_Man_t* p, int i);
char* Abc_ObjName(Abc_Obj_t*);
char* Extra_UtilStrsav(const char*);
void Hop_ObjPrintVerilog(FILE*, Hop_Obj_t*, Vec_Vec_t*, int, int);
}

namespace hal
{
    namespace
    {
        /// `ContextABC` manages memory and lifetime of the global ABC context.
        ///
        /// # Developer Note
        /// `ContextABC` behaves as a singleton as this variable is (and should be)
        /// just initialized once statically within the abc_simplfication function.
        /// Note that we opted for this solution, rather than a real singleton be-
        /// havior as we cannot easily cleanup the singleton within HAL.
        class ContextABC final
        {
        public:
            /// Initializes the global ABC context.
            ContextABC()
            {
                Abc_Start();
            }

            /// Deinitialize the global ABC context.
            ~ContextABC()
            {
                Abc_Stop();
            }
        };

        /// Copied from "src/base/abc/abc.h", see above for link to ABC's repository
        Abc_Obj_t* Abc_ObjFanin(Abc_Obj_t* pObj, int i)
        {
            return (Abc_Obj_t*)pObj->pNtk->vObjs->pArray[pObj->vFanins.pArray[i]];
        }

        /// Copied from "src/base/io/ioWriteVerilog.c", see above for link to ABC's repository
        char* Io_WriteVerilogGetName(char* pName)
        {
            static char Buffer[500];
            int i, Length = strlen(pName);
            if (pName[0] < '0' || pName[0] > '9')
            {
                for (i = 0; i < Length; i++)
                    if (!((pName[i] >= 'a' && pName[i] <= 'z') || (pName[i] >= 'A' && pName[i] <= 'Z') || (pName[i] >= '0' && pName[i] <= '9') || pName[i] == '_'))
                        break;
                if (i == Length)
                    return pName;
            }
            // create Verilog style name
            Buffer[0] = '\\';
            for (i = 0; i < Length; i++)
                Buffer[i + 1] = pName[i];
            Buffer[Length + 1] = ' ';
            Buffer[Length + 2] = 0;
            return Buffer;
        }

        /// Copied from "src/misc/vec/vecVec.h", see above for link to ABC's repository
        Vec_Vec_t* Vec_VecAlloc(int nCap)
        {
            Vec_Vec_t* p;
            p = ((Vec_Vec_t*)malloc(sizeof(Vec_Vec_t) * (1)));
            if (nCap > 0 && nCap < 8)
                nCap = 8;
            p->nSize  = 0;
            p->nCap   = nCap;
            p->pArray = p->nCap ? (void**)malloc(sizeof(void*) * p->nCap) : NULL;
            return p;
        }

        /// Copied from "src/misc/vec/vecPtr.h", see above for link to ABC's repository
        ///
        /// # Change Note
        /// We included the #pragma check, defintion, and undefinition of ABC_FREE
        /// directly within the function to minimize the scope of the macro. Note
        /// that the ABC_FREE macro itself is defined in "src/misc/util/abc_global.h"
        void Vec_PtrFree(Vec_Ptr_t* p)
        {
#define ABC_FREE(obj) ((obj) ? (free((char*)(obj)), (obj) = 0) : 0)
            ABC_FREE(p->pArray);
            ABC_FREE(p);
#undef ABC_FREE
        }

        /// Copied from "src/misc/vec/vecVec.h", see above for link to ABC's repository
        void Vec_VecFree(Vec_Vec_t* p)
        {
            Vec_Ptr_t* vVec;
            int i;
            for (i = 0; (i < p->nSize) && (((vVec) = (Vec_Ptr_t*)p->pArray[i]), 1); i++)
            {
                if (vVec)
                {
                    Vec_PtrFree(vVec);
                }
            }

            Vec_PtrFree((Vec_Ptr_t*)p);
        }

        /**
         * Helper function to generate single-bit output variables for a multi-bit
         * Boolean function with >= 1 output bits.
         * 
         * @param[in] function - Boolean function.
         * @return List of output variables to identify each bit.
         */
        std::vector<std::string> get_output_variables(const BooleanFunction& function)
        {
            std::vector<std::string> outputs;
            outputs.reserve(function.size());
            for (auto bit = 0u; bit < function.size(); bit++)
            {
                outputs.emplace_back("output_" + std::to_string(bit));
            }
            return outputs;
        }

        /** 
         * Short-hand function to access a sliced-index of the Boolean function.
         * 
         * @param[in] function - Boolean function.
         * @param[in] index - Bit index of the Boolean function.
         * @returns Sliced and simplified Boolean function on success, error otherwise.
         */
        Result<BooleanFunction> slice_at(const BooleanFunction& function, const u16 index)
        {
            // (1) setup the sliced Boolean function
            if (auto res =
                    BooleanFunction::Slice(function.clone(), BooleanFunction::Index(index, function.size()), BooleanFunction::Index(index, function.size()), 1).map<BooleanFunction>([](const auto& f) {
                        // (2) perform a local simplification in order to remove unnecessary slices
                        return Simplification::local_simplification(f);
                    });
                res.is_error())
            {
                return ERR(res.get_error());
            }
            else
            {
                return res;
            }
        }

        /**
         * Translates a `BooleanFunction` to the ABC network representation.
         * 
         * @param[in] function - Boolean function to translate.
         * @returns Ok() and ABC network on success, Err() otherwise.
         */
        Result<Abc_Ntk_t*> translate_to_abc(const BooleanFunction& function)
        {
            /**
             * Local translation helper to translate a given node and its operands.
             *
             * @param[in] node - Boolean function node.
             * @param[in] operands - Boolean function node operands.
             * @param[in] input2abc - Maps each Boolean function inputs to an ABC object.
             * @param[in] network - ABC network.
             * @returns Ok() and ABC object on success, Err() otherwise.
             */
            auto translate = [](const auto& node, auto&& operands, const auto& input2abc, auto network) -> Result<Abc_Obj_t*> {
                // (1) short-hand check that we read correct operand data below
                if (node.get_arity() != operands.size())
                {
                    return ERR("could not translate Boolean function to ABC notation: invalid number of parameters");
                }

                /// # Developer Note
                /// Since we use ABC's And-Inverter-Graph (AIG) representation for
                /// our Boolean function simplification, we so far only focus the
                /// translation of And, Not, Or, Xor and Variable nodes.
                /// In case we want to extend the functionality (e.g., for equality
                /// Boolean function nodes), we have to model the node semantics
                /// using the AigAnd, AigOr, AigXor (and AigConst1) interface, see
                /// BooleanFunction::NodeType::Not where we had to resort to an XOR
                /// with a logical 1 to mimic a NOT semantics.
                switch (node.type)
                {
                    case BooleanFunction::NodeType::Variable:
                        return OK(input2abc.at(node.variable));

                    case BooleanFunction::NodeType::And:
                        return OK(Abc_AigAnd((Abc_Aig_t*)network->pManFunc, operands[0], operands[1]));
                    case BooleanFunction::NodeType::Not:
                        return OK(Abc_AigXor((Abc_Aig_t*)network->pManFunc, operands[0], Abc_AigConst1(network)));
                    case BooleanFunction::NodeType::Or:
                        return OK(Abc_AigOr((Abc_Aig_t*)network->pManFunc, operands[0], operands[1]));
                    case BooleanFunction::NodeType::Xor:
                        return OK(Abc_AigXor((Abc_Aig_t*)network->pManFunc, operands[0], operands[1]));

                    default:
                        return ERR("could not translate Boolean function to ABC notation: not implemented for given node type");
                }
            };

            // (1) initialze a new ABC network with structured hashing
            auto network = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);

            // (2) initialize Boolean function input variables as ABC network objects
            std::map<std::string, Abc_Obj_t*> input2abc;
            for (auto variable : function.get_variable_names())
            {
                auto object = Abc_NtkCreateObj(network, ABC_OBJ_PI);
                Abc_ObjAssignName(object, variable.data(), NULL);

                input2abc[variable] = object;
            }

            const auto output_variables = get_output_variables(function);
            for (auto i = 0u; i < output_variables.size(); i++)
            {
                auto status = slice_at(function, i);
                if (status.is_error())
                {
                    return ERR_APPEND(status.get_error(),
                                      "could not translate Boolean function to ABC notation: unable to slice Boolean function'" + function.to_string() + "' at index " + std::to_string(i));
                }

                // (3) translation from Boolean function nodes to ABC network objects
                std::vector<Abc_Obj_t*> stack;
                for (const auto& node : status.get().get_nodes())
                {
                    std::vector<Abc_Obj_t*> operands;
                    std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
                    stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

                    auto translation = translate(node, std::move(operands), input2abc, network);
                    if (translation.is_ok())
                    {
                        stack.emplace_back(translation.get());
                    }
                    else
                    {
                        return ERR(translation.get_error());
                    }
                }

                // (4) check that we have exactely one remaining entry on the stack
                if (stack.size() != 1)
                {
                    return ERR("could not translate Boolean function to ABC notation: number of elements remaining on the stack is not 1");
                }

                // (5) define an output variable to the network
                auto output = Abc_NtkCreateObj(network, ABC_OBJ_PO);
                Abc_ObjAssignName(output, strdup(output_variables[i].data()), NULL);
                Abc_ObjAddFanin(output, stack.back());
            }

            // (6) internal ABC cleanup to remove any dangling object
            Abc_AigCleanup((Abc_Aig_t*)network->pManFunc);

            // (7) check whether the network construction is successful
            if (!Abc_NtkCheck(network))
            {
                Abc_NtkDelete(network);
                return ERR("could not translate Boolean function to ABC notation: AbcNtkCheck() failed");
            }

            return OK(network);
        }

        /**
         * Performs the Boolean function simplification using ABC.
         * 
         * @param[in] network - ABC network that represents the Boolean function.
         * @returns Ok() on success, Err() otherwise.
         */
        Result<std::monostate> simplify(Abc_Ntk_t* network)
        {
            // (1) since we apply the ABC simplification using the command dispatch
            //     functionality, we have to set the network within the global frame
            auto abc = Abc_FrameGetGlobalFrame();

            Abc_FrameReplaceCurrentNetwork(abc, network);
            Abc_FrameClearVerifStatus(abc);

            // (2) perform the Boolean function simplification using a series of ABC
            //     simplification transformations on the internal graph

            // TODO(@nalbartus):
            // Analyze which number of calls are minimal to achieve a "good enough"
            // simplification / optimization result for further analysis
            Cmd_CommandExecute(abc, R"#(
                fraig;
                balance;
                rewrite -l;
                refactor -l;
                balance;
                rewrite -l;
                rewrite -lz;
                balance; 
                refactor -lz;
                rewrite -lz;
                balance;
                rewrite -lz;
            )#");

            return OK({});
        }

        /**
         * Translates the simplified ABC network (stored in the global ABC context)
         * as a list of (intermediate) variable assignments in the following form:
         *      
         * # Example
         * 
         *       assign new_n5_ = ~A & ~B;
         *       assign output_0 = C | ~new_n5_;
         * 
         * Note that the assignment above represents "C | ~(~A & ~B)" with "new_n5_"
         * as a temporary variable.
         * 
         * @returns Ok() and assignments as string on success, Err() otherwise.
         */
        Result<std::string> translate_from_abc()
        {
            std::stringstream ss;

            // (1) translate the simplified network back to a netlist
            auto network = Abc_NtkToNetlist(Abc_FrameGetGlobalFrame()->pNtkCur);
            if (!(network->ntkFunc == ABC_FUNC_AIG) && !(network->ntkFunc == ABC_FUNC_MAP))
            {
                Abc_NtkToAig(network);
            }

            // (2) iterate each the ABC graph to assemble a list of structured
            //     Verilog assignments that implement the simplfied function
            auto vLevels = Vec_VecAlloc(10);
            for (int i = 0; i < network->vObjs->nSize; i++)
            {
                /// # Developer Note
                /// The code within the for loop is adapted based on ABC's verilog
                /// writer Io_WriteVerilogObjects() in src/base/io/ioWriteVerilog.c,
                /// in particular the else case as the netlist has no mapping.
                /// However, in order to re-use the FILE* based interface, we use an
                /// in-memory implementation to prevent a file read/write for each
                /// Boolean function simplification.

                auto node = (Abc_Obj_t*)network->vObjs->pArray[i];
                if ((node == NULL) || !(node->Type == ABC_OBJ_NODE))
                {
                    continue;
                }
                if (node->fPersist)
                {
                    return ERR("could not translate Boolean function from ABC notation: 'node->fPersist' is set within ABC context");
                }

                Abc_Obj_t* pFanin = nullptr;
                for (int j = 0; (j < node->vFanins.nSize) && (((pFanin) = Abc_ObjFanin(node, j)), 1); j++)
                {
                    Hop_IthVar((Hop_Man_t*)network->pManFunc, j)->pData = Extra_UtilStrsav(Io_WriteVerilogGetName(Abc_ObjName(pFanin)));
                }

                // (2.1) compute the left-hand side variable name
                const auto lhs = std::string(Io_WriteVerilogGetName(Abc_ObjName((Abc_Obj_t*)node->pNtk->vObjs->pArray[node->vFanouts.pArray[0]])));

                // (2.2) compute the right-hand side Boolean function expression via
                //       in-memory FILE* to mimic the original interface without the
                //       use of an actual file read/write operation
                char* rhs{};
                size_t len{};

                auto stream = open_memstream(&rhs, &len);
                Hop_ObjPrintVerilog(stream, (Hop_Obj_t*)node->pData, vLevels, 0, 0);
                fflush(stream);

                // (2.3) assemble "assign $(LHS) = $(RHS);"
                ss << "assign " << lhs << " = " << rhs << ";" << std::endl;

                fclose(stream);
            }
            Vec_VecFree(vLevels);

            return OK(ss.str());
        }

        /** 
         * Translates a structured Verilog file to a Boolean function.
         * 
         * @param[in] verilog - Verilog data that represents a list of assignments
         * @param[in] function Input Boolean function (not simplified).
         * @returns Simplified Boolean function on success, error otherwise.
         */
        Result<BooleanFunction> translate_from_verilog(const std::string& verilog, const BooleanFunction& function)
        {
            /**
             * Translates an Verilog statement such as "assign new_n8_ = ~I1 & ~I3;"
             * into the respective left-hand and right-hand side Boolean functions.
             * 
             * @param[in] assignment - Verilog assignment statement.
             * @returns Left-hand and right-hand Boolean functions on success, error otherwise.
             */
            auto parse_assignment = [](const auto& assignment) -> Result<std::tuple<BooleanFunction, BooleanFunction>> {
                Result<BooleanFunction> lhs = ERR(""), rhs = ERR("");

                namespace x3 = boost::spirit::x3;

                ////////////////////////////////////////////////////////////////////////
                // Actions
                ////////////////////////////////////////////////////////////////////////

                const auto LHSAction = [&lhs](auto& ctx) { lhs = BooleanFunction::from_string(_attr(ctx)); };
                const auto RHSAction = [&rhs](auto& ctx) { rhs = BooleanFunction::from_string(_attr(ctx)); };

                ////////////////////////////////////////////////////////////////////////
                // Rules
                ////////////////////////////////////////////////////////////////////////

                const auto EndOfLineRule = x3::lit(";") >> *x3::space;
                const auto RHSRule       = x3::lexeme[*x3::char_("a-zA-Z0-9_+*~|&!'()[]\\ ")][RHSAction];
                const auto EqualSignRule = *x3::space >> x3::lit("=") >> *x3::space;
                const auto LHSRule       = x3::lexeme[*x3::char_("a-zA-Z0-9_")][LHSAction];
                const auto AssignRule    = *x3::space >> x3::lit("assign") >> *x3::space;

                // (1) parse the assignment to left-hand and right-hand side
                auto iter     = assignment.begin();
                const auto ok = x3::phrase_parse(iter,
                                                 assignment.end(),
                                                 ////////////////////////////////////////////////////////////////////
                                                 // Parsing Expression Grammar
                                                 ////////////////////////////////////////////////////////////////////
                                                 AssignRule >> LHSRule >> EqualSignRule >> RHSRule >> EndOfLineRule,
                                                 // we use an invalid a.k.a. non-printable ASCII character in order
                                                 // to prevent the skipping of space characters as they are defined
                                                 // as skipper within a Boolean function and operation
                                                 x3::char_(0x00));

                if (!ok || (iter != assignment.end()))
                {
                    return ERR("could not parse assignment from Verilog notation: '" + assignment + "' (remaining '" + std::string(iter, assignment.end()) + "')");
                }

                if (lhs.is_error())
                {
                    return ERR_APPEND(lhs.get_error(), "cannot parse assignment from Verilog notation: unable to translate left side of assignment '" + assignment + "' into a Boolean function");
                }
                if (rhs.is_error())
                {
                    return ERR_APPEND(rhs.get_error(), "cannot parse assignment from Verilog notation: unable to translate right side of assignment '" + assignment + "' into a Boolean function");
                }

                return OK({lhs.get(), rhs.get()});
            };

            std::map<BooleanFunction, BooleanFunction> assignments;

            // (1) translate each Verilog assignment of the form "assign ... = ...;"
            std::istringstream data(verilog);
            std::string line;
            while (std::getline(data, line))
            {
                if (auto assignment = parse_assignment(line); assignment.is_ok())
                {
                    auto [lhs, rhs]  = assignment.get();
                    assignments[lhs] = rhs;
                }
            }

            // (2) check that each output variable is defined within the assignments
            const auto output_variables = get_output_variables(function);
            for (const auto& output_variable : output_variables)
            {
                if (assignments.find(BooleanFunction::Var(output_variable)) == assignments.end())
                {
                    return ERR("could not parse assignment from Verilog notation: unable to simplify Boolean function'" + function.to_string() + "' as output variable is not defined in Verilog");
                }
            }

            // (3) recursively replace the variables for each output Boolean function
            //     until only input variables of the original function are present in
            //     the function
            const auto inputs = function.get_variable_names();
            for (const auto& output_variable : output_variables)
            {
                auto output = BooleanFunction::Var(output_variable);

                // in order to prevent infinite-loops by unsupported inputs such as
                // cyclic replacement dependencies, we only replace |assignments| times
                auto counter = 0u;
                while ((assignments[output].get_variable_names() != inputs) && (counter++ < assignments.size()))
                {
                    for (const auto& tmp : assignments[output].get_variable_names())
                    {
                        if (inputs.find(tmp) != inputs.end())
                        {
                            continue;
                        }
                        auto simplified = assignments[output].substitute(tmp, assignments[BooleanFunction::Var(tmp)]);
                        if (simplified.is_error())
                        {
                            return ERR(simplified.get_error());
                        }

                        assignments[output] = simplified.get();
                    }
                }
            }

            // (4) validate the all outputs only contain input variables
            for (const auto& output_variable : output_variables)
            {
                for (const auto& input : assignments[BooleanFunction::Var(output_variable)].get_variable_names())
                {
                    if (inputs.find(input) == inputs.end())
                    {
                        return ERR("could not parse assignment from Verilog notation: unable to replace output variable '" + output_variable + "' as it contains temporary variable '" + input + "'");
                    }
                }
            }

            // (5) concatenate all output Boolean functions into a single function
            auto state = assignments[BooleanFunction::Var(output_variables[0])];
            for (auto i = 1u; i < function.size(); i++)
            {
                auto concat = BooleanFunction::Concat(assignments[BooleanFunction::Var(output_variables[i])].clone(), state.clone(), state.size() + 1);

                if (concat.is_ok())
                {
                    state = concat.get();
                }
                else
                {
                    return concat;
                }
            }
            return OK(state);
        }
    }    // namespace

    Result<BooleanFunction> Simplification::abc_simplification(const BooleanFunction& function)
    {
        // # Developer Note
        // In order to apply a global optimization to the Boolean function, we
        // leverage the rich-featured and robust implementations of ABC [1]. To
        // this end, we first translate the Boolean function into a subset of
        // ABC's and-inverter graph structures (e.g., ABC cannot handle arith-
        // metic such as '+' and we have to translate multi-bit vectors into
        // single bit-vectors). We then use the rewriting, balancing and re-
        // factoring in order to minimize the Boolean function and then read-
        // back the graph back into our Boolean function representation.
        //
        // [1] https://people.eecs.berkeley.edu/~alanmi/abc/
        static ContextABC context;

        // (1) check whether the Boolean function can be simplified using ABC
        using NodeType = BooleanFunction::NodeType;

        static const std::set<u16> valid_abc_node_types({
            NodeType::And,
            NodeType::Not,
            NodeType::Or,
            NodeType::Xor,
            NodeType::Variable,
        });

        if (auto nodes = function.get_nodes();
            std::any_of(nodes.begin(), nodes.end(), [](auto node) { return valid_abc_node_types.find(node.type) == valid_abc_node_types.end(); }) || function.get_variable_names().empty())
        {
            return OK(function.clone());
        }

        // (2) since the simplification and translations require access to the
        //     ABC global frame, we have to ensure an exclusive access in case
        //     Boolean function simplifications are executed in parallel
        static std::mutex mutex;
        mutex.lock();

        auto status = translate_to_abc(function).map<std::monostate>([](const auto& network) { return simplify(network); });

        if (status.is_error())
        {
            return ERR_APPEND(status.get_error(), "could not simplyfy Boolean function using ABC: unable to translate & simplify Boolean function '" + function.to_string() + "'");
        }

        // (3) translate the ABC graph back into a Boolean function
        auto translated_function = translate_from_abc().map<BooleanFunction>([&function](const auto& verilog) { return translate_from_verilog(verilog, function); });

        mutex.unlock();

        if (translated_function.is_ok())
        {
            return translated_function;
        }
        else
        {
            return OK(function.clone());
        }
    }
}    // namespace hal
