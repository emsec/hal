#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/gate_decorator_system.h"
#include "plugin_gate_decorators.h"

#include "core/log.h"
#include "netlist/gate.h"

namespace bdd_NangateOpenCellLibrary_helper
{
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND2_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (A1 & A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND2_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (A1 & A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND2_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (A1 & A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND3_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = ((A1 & A2) & A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND3_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = ((A1 & A2) & A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND3_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = ((A1 & A2) & A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND4_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (((A1 & A2) & A3) & A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"])) & *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND4_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (((A1 & A2) & A3) & A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"])) & *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AND4_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (((A1 & A2) & A3) & A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"])) & *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI211_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 & C2) | B) | A)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])) | *(input_pin_type_to_bdd["B"])) | *(input_pin_type_to_bdd["A"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI211_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 & C2) | B) | A)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])) | *(input_pin_type_to_bdd["B"])) | *(input_pin_type_to_bdd["A"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI211_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(!(!(((C1 & C2) | B) | A)))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(bdd_not(bdd_not(((*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])) | *(input_pin_type_to_bdd["B"])) | *(input_pin_type_to_bdd["A"]))));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI21_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A | (B1 & B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI21_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A | (B1 & B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI21_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A | (B1 & B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI221_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 & C2) | A) | (B1 & B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0 =
            bdd_not(((*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])) | *(input_pin_type_to_bdd["A"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])));
        result["ZN"] = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI221_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 & C2) | A) | (B1 & B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0 =
            bdd_not(((*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])) | *(input_pin_type_to_bdd["A"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])));
        result["ZN"] = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI221_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(!(!(((C1 & C2) | A) | (B1 & B2))))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(
            bdd_not(bdd_not(((*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])) | *(input_pin_type_to_bdd["A"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])))));
        result["ZN"] = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI222_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 & A2) | (B1 & B2)) | (C1 & C2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])))
                                | (*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI222_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 & A2) | (B1 & B2)) | (C1 & C2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])))
                                | (*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI222_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(!(!(((A1 & A2) | (B1 & B2)) | (C1 & C2))))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(bdd_not(bdd_not(((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])))
                                                | (*(input_pin_type_to_bdd["C1"]) & *(input_pin_type_to_bdd["C2"])))));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI22_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 & A2) | (B1 & B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI22_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 & A2) | (B1 & B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_AOI22_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 & A2) | (B1 & B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) | (*(input_pin_type_to_bdd["B1"]) & *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_BUF_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_BUF_X16(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_BUF_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_BUF_X32(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_BUF_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_BUF_X8(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_CLKBUF_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_CLKBUF_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_CLKBUF_X3(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_FA_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : CO = ((A & B) | (CI & (A | B)))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["A"]) & *(input_pin_type_to_bdd["B"])) | (*(input_pin_type_to_bdd["CI"]) & (*(input_pin_type_to_bdd["A"]) | *(input_pin_type_to_bdd["B"]))));
        result["CO"]      = bdd_output_0;

        // function : S = (CI ^ (A ^ B))
        auto bdd_output_1 = std::make_shared<bdd>();
        *bdd_output_1     = (*(input_pin_type_to_bdd["CI"]) ^ (*(input_pin_type_to_bdd["A"]) ^ *(input_pin_type_to_bdd["B"])));
        result["S"]       = bdd_output_1;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_HA_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : CO = (A & B)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A"]) & *(input_pin_type_to_bdd["B"]));
        result["CO"]      = bdd_output_0;

        // function : S = (A ^ B)
        auto bdd_output_1 = std::make_shared<bdd>();
        *bdd_output_1     = (*(input_pin_type_to_bdd["A"]) ^ *(input_pin_type_to_bdd["B"]));
        result["S"]       = bdd_output_1;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_INV_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_INV_X16(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_INV_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_INV_X32(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_INV_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_INV_X8(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_LOGIC0_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = 0
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_false();
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_LOGIC1_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = 1
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_true();
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_MUX2_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = ((S & B) | (A & !S))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["S"]) & *(input_pin_type_to_bdd["B"])) | (*(input_pin_type_to_bdd["A"]) & bdd_not(*(input_pin_type_to_bdd["S"]))));
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_MUX2_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = ((S & B) | (A & !S))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["S"]) & *(input_pin_type_to_bdd["B"])) | (*(input_pin_type_to_bdd["A"]) & bdd_not(*(input_pin_type_to_bdd["S"]))));
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND2_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A1 & A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND2_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A1 & A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND2_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A1 & A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND3_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 & A2) & A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND3_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 & A2) & A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND3_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 & A2) & A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND4_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 & A2) & A3) & A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"])) & *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND4_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 & A2) & A3) & A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"])) & *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NAND4_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 & A2) & A3) & A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) & *(input_pin_type_to_bdd["A2"])) & *(input_pin_type_to_bdd["A3"])) & *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR2_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A1 | A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR2_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A1 | A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR2_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A1 | A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR3_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 | A2) | A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR3_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 | A2) | A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR3_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 | A2) | A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR4_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 | A2) | A3) | A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"])) | *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR4_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 | A2) | A3) | A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"])) | *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_NOR4_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 | A2) | A3) | A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"])) | *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI211_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 | C2) & A) & B)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])) & *(input_pin_type_to_bdd["A"])) & *(input_pin_type_to_bdd["B"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI211_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 | C2) & A) & B)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])) & *(input_pin_type_to_bdd["A"])) & *(input_pin_type_to_bdd["B"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI211_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 | C2) & A) & B)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])) & *(input_pin_type_to_bdd["A"])) & *(input_pin_type_to_bdd["B"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI21_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A & (B1 | B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI21_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A & (B1 | B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI21_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A & (B1 | B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI221_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 | C2) & A) & (B1 | B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0 =
            bdd_not(((*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])) & *(input_pin_type_to_bdd["A"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])));
        result["ZN"] = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI221_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((C1 | C2) & A) & (B1 | B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0 =
            bdd_not(((*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])) & *(input_pin_type_to_bdd["A"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])));
        result["ZN"] = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI221_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(!(!(((C1 | C2) & A) & (B1 | B2))))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(
            bdd_not(bdd_not(((*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])) & *(input_pin_type_to_bdd["A"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])))));
        result["ZN"] = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI222_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 | A2) & (B1 | B2)) & (C1 | C2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])))
                                & (*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI222_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 | A2) & (B1 | B2)) & (C1 | C2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])))
                                & (*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI222_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(!(!(((A1 | A2) & (B1 | B2)) & (C1 | C2))))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(bdd_not(bdd_not(((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])))
                                                & (*(input_pin_type_to_bdd["C1"]) | *(input_pin_type_to_bdd["C2"])))));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI22_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 | A2) & (B1 | B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI22_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 | A2) & (B1 | B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI22_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !((A1 | A2) & (B1 | B2))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) & (*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OAI33_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(((A1 | A2) | A3) & ((B1 | B2) | B3))
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"]))
                                & ((*(input_pin_type_to_bdd["B1"]) | *(input_pin_type_to_bdd["B2"])) | *(input_pin_type_to_bdd["B3"])));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR2_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (A1 | A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR2_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (A1 | A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR2_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (A1 | A2)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR3_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = ((A1 | A2) | A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR3_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = ((A1 | A2) | A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR3_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = ((A1 | A2) | A3)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = ((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR4_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (((A1 | A2) | A3) | A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"])) | *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR4_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (((A1 | A2) | A3) | A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"])) | *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_OR4_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = (((A1 | A2) | A3) | A4)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (((*(input_pin_type_to_bdd["A1"]) | *(input_pin_type_to_bdd["A2"])) | *(input_pin_type_to_bdd["A3"])) | *(input_pin_type_to_bdd["A4"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_TBUF_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_TBUF_X16(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_TBUF_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_TBUF_X4(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_TBUF_X8(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = A
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = *(input_pin_type_to_bdd["A"]);
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_TINV_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !I
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["I"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_XNOR2_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A ^ B)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]) ^ *(input_pin_type_to_bdd["B"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_XNOR2_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : ZN = !(A ^ B)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = bdd_not(*(input_pin_type_to_bdd["A"]) ^ *(input_pin_type_to_bdd["B"]));
        result["ZN"]      = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_XOR2_X1(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = (A ^ B)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A"]) ^ *(input_pin_type_to_bdd["B"]));
        result["Z"]       = bdd_output_0;
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_NangateOpenCellLibrary_XOR2_X2(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        UNUSED(gate);
        std::map<std::string, std::shared_ptr<bdd>> result;

        // function : Z = (A ^ B)
        auto bdd_output_0 = std::make_shared<bdd>();
        *bdd_output_0     = (*(input_pin_type_to_bdd["A"]) ^ *(input_pin_type_to_bdd["B"]));
        result["Z"]       = bdd_output_0;
        return result;
    }
}    // namespace bdd_NangateOpenCellLibrary_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators = {
    {"AND2_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND2_X1},     {"AND2_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND2_X2},
    {"AND2_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND2_X4},     {"AND3_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND3_X1},
    {"AND3_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND3_X2},     {"AND3_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND3_X4},
    {"AND4_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND4_X1},     {"AND4_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND4_X2},
    {"AND4_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AND4_X4},     {"AOI211_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI211_X1},
    {"AOI211_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI211_X2}, {"AOI211_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI211_X4},
    {"AOI21_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI21_X1},   {"AOI21_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI21_X2},
    {"AOI21_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI21_X4},   {"AOI221_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI221_X1},
    {"AOI221_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI221_X2}, {"AOI221_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI221_X4},
    {"AOI222_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI222_X1}, {"AOI222_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI222_X2},
    {"AOI222_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI222_X4}, {"AOI22_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI22_X1},
    {"AOI22_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI22_X2},   {"AOI22_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_AOI22_X4},
    {"BUF_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_BUF_X1},       {"BUF_X16", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_BUF_X16},
    {"BUF_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_BUF_X2},       {"BUF_X32", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_BUF_X32},
    {"BUF_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_BUF_X4},       {"BUF_X8", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_BUF_X8},
    {"CLKBUF_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_CLKBUF_X1}, {"CLKBUF_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_CLKBUF_X2},
    {"CLKBUF_X3", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_CLKBUF_X3}, {"FA_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_FA_X1},
    {"HA_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_HA_X1},         {"INV_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_INV_X1},
    {"INV_X16", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_INV_X16},     {"INV_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_INV_X2},
    {"INV_X32", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_INV_X32},     {"INV_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_INV_X4},
    {"INV_X8", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_INV_X8},       {"LOGIC0_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_LOGIC0_X1},
    {"LOGIC1_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_LOGIC1_X1}, {"MUX2_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_MUX2_X1},
    {"MUX2_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_MUX2_X2},     {"NAND2_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND2_X1},
    {"NAND2_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND2_X2},   {"NAND2_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND2_X4},
    {"NAND3_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND3_X1},   {"NAND3_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND3_X2},
    {"NAND3_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND3_X4},   {"NAND4_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND4_X1},
    {"NAND4_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND4_X2},   {"NAND4_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NAND4_X4},
    {"NOR2_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR2_X1},     {"NOR2_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR2_X2},
    {"NOR2_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR2_X4},     {"NOR3_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR3_X1},
    {"NOR3_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR3_X2},     {"NOR3_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR3_X4},
    {"NOR4_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR4_X1},     {"NOR4_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR4_X2},
    {"NOR4_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_NOR4_X4},     {"OAI211_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI211_X1},
    {"OAI211_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI211_X2}, {"OAI211_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI211_X4},
    {"OAI21_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI21_X1},   {"OAI21_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI21_X2},
    {"OAI21_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI21_X4},   {"OAI221_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI221_X1},
    {"OAI221_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI221_X2}, {"OAI221_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI221_X4},
    {"OAI222_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI222_X1}, {"OAI222_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI222_X2},
    {"OAI222_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI222_X4}, {"OAI22_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI22_X1},
    {"OAI22_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI22_X2},   {"OAI22_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI22_X4},
    {"OAI33_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OAI33_X1},   {"OR2_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR2_X1},
    {"OR2_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR2_X2},       {"OR2_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR2_X4},
    {"OR3_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR3_X1},       {"OR3_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR3_X2},
    {"OR3_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR3_X4},       {"OR4_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR4_X1},
    {"OR4_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR4_X2},       {"OR4_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_OR4_X4},
    {"TBUF_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_TBUF_X1},     {"TBUF_X16", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_TBUF_X16},
    {"TBUF_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_TBUF_X2},     {"TBUF_X4", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_TBUF_X4},
    {"TBUF_X8", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_TBUF_X8},     {"TINV_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_TINV_X1},
    {"XNOR2_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_XNOR2_X1},   {"XNOR2_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_XNOR2_X2},
    {"XOR2_X1", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_XOR2_X1},     {"XOR2_X2", bdd_NangateOpenCellLibrary_helper::get_bdd_NangateOpenCellLibrary_XOR2_X2}};

std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_NangateOpenCellLibrary(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
{
    auto type = g->get_type();

    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        log_error("netlist.decorator", "not implemented reached for gate type '{}'.", type);
        return std::map<std::string, std::shared_ptr<bdd>>();
    }

    gate_decorator_system::bdd_decorator_generator generator = m_bbd_generators.at(type);
    return generator(g, input_pin_type_to_bdd);
}

bool plugin_gate_decorators::bdd_availability_tester_NangateOpenCellLibrary(std::shared_ptr<gate> g)
{
    auto type = g->get_type();

    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        return false;
    }
    return true;
}
