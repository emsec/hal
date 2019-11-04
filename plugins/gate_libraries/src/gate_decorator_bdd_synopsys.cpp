#include "plugin_gate_decorators.h"
#include "gate_decorator_system/gate_decorator_system.h"
#include "gate_decorator_system/decorators/gate_decorator_bdd.h"

#include "core/log.h"
#include "netlist/gate.h"

namespace bdd_synopsys_helper
{
    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_xor(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "XOR2X1" || gate->get_type() == "XOR2X2")
        {
            //function : "(IN1^IN2)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) ^ *(input_pin_type_to_bdd["IN2"]);
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "XOR3X1" || gate->get_type() == "XOR3X2")
        {
            //function : "(IN1^IN2^IN3)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) ^ *(input_pin_type_to_bdd["IN2"]) ^ *(input_pin_type_to_bdd["IN3"]);
            result["Q"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_xnor(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "XNOR2X1" || gate->get_type() == "XNOR2X2")
        {
            //function : "(IN1^IN2)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not(*(input_pin_type_to_bdd["IN1"]) ^ *(input_pin_type_to_bdd["IN2"]));
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "XNOR3X1" || gate->get_type() == "XNOR3X2")
        {
            //function : "(IN1^IN2^IN3)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not(*(input_pin_type_to_bdd["IN1"]) ^ *(input_pin_type_to_bdd["IN2"]) ^ *(input_pin_type_to_bdd["IN3"]));
            result["Q"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_and(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "AND2X1" || gate->get_type() == "AND2X2")
        {
            //function : "(IN1 IN2)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"]);
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "AND3X1" || gate->get_type() == "AND3X2" || gate->get_type() == "AND3X4")
        {
            //function : "(IN1 IN2 IN3)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"]) & *(input_pin_type_to_bdd["IN3"]);
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "AND4X1" || gate->get_type() == "AND4X2" || gate->get_type() == "AND4X4")
        {
            //function : "(IN1 IN2 IN3 IN4)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"]) & *(input_pin_type_to_bdd["IN3"]) & *(input_pin_type_to_bdd["IN4"]);
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "ISOLANDX1" || gate->get_type() == "ISOLANDX2" || gate->get_type() == "ISOLANDX4" || gate->get_type() == "ISOLANDX8")
        {
            //function : "(D ISO)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["D"]) & *(input_pin_type_to_bdd["ISO"]);
            result["Q"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_andorinv(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "AOI21X1" || gate->get_type() == "AOI21X2")
        {
            //function : "(!((IN1 IN2)+IN3))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not((*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) | *(input_pin_type_to_bdd["IN3"]));
            result["QN"]    = bdd_output;
        }
        if (gate->get_type() == "AOI22X1" || gate->get_type() == "AOI22X2")
        {
            //function : "(!((IN1 IN2)+(IN3 IN4)))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not((*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) | (*(input_pin_type_to_bdd["IN3"]) & *(input_pin_type_to_bdd["IN4"])));
            result["QN"]    = bdd_output;
        }
        if (gate->get_type() == "AOI221X1" || gate->get_type() == "AOI221X2")
        {
            //function : "(!((IN1 IN2)+(IN3 IN4)+ IN5)))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output =
                bdd_not((*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) | (*(input_pin_type_to_bdd["IN3"]) & *(input_pin_type_to_bdd["IN4"])) | *(input_pin_type_to_bdd["IN5"]));
            result["QN"] = bdd_output;
        }
        if (gate->get_type() == "AOI222X1" || gate->get_type() == "AOI222X2")
        {
            //function : "(!((IN1 IN2)+(IN3 IN4)+(IN5 IN6)))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not((*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) | (*(input_pin_type_to_bdd["IN3"]) & *(input_pin_type_to_bdd["IN4"]))
                                  | (*(input_pin_type_to_bdd["IN5"]) & *(input_pin_type_to_bdd["IN6"])));
            result["QN"]    = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_andor(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "AO21X1" || gate->get_type() == "AO21X2")
        {
            //function : "((IN1 IN2)+IN3)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = (*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) | *(input_pin_type_to_bdd["IN3"]);
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "AO22X1" || gate->get_type() == "AO22X2")
        {
            //function : "((IN1 IN2)+(IN3 IN4))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = (*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) | (*(input_pin_type_to_bdd["IN3"]) & *(input_pin_type_to_bdd["IN4"]));
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "AO221X1" || gate->get_type() == "AO221X2")
        {
            //function : "((IN1 IN2)+(IN3 IN4)+ IN5)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output = (*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) | (*(input_pin_type_to_bdd["IN3"]) & *(input_pin_type_to_bdd["IN4"])) | *(input_pin_type_to_bdd["IN5"]);
            result["Q"] = bdd_output;
        }
        if (gate->get_type() == "AO222X1" || gate->get_type() == "AO222X2")
        {
            //function : "((IN1 IN2)+(IN3 IN4)+(IN5 IN6)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = (*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) | (*(input_pin_type_to_bdd["IN3"]) & *(input_pin_type_to_bdd["IN4"]))
                          | (*(input_pin_type_to_bdd["IN5"]) & *(input_pin_type_to_bdd["IN6"]));
            result["Q"] = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_inv(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;

        std::set<std::string> gate_types = {
            "INVX0",
            "INVX1",
            "INVX2",
            "INVX4",
            "INVX8",
            "INVX16",
            "INVX32",
            "IBUFFX2",
            "IBUFFX4",
            "IBUFFX8",
            "IBUFFX16",
            "IBUFFX32",
        };

        if (gate_types.find(gate->get_type()) != gate_types.end())
        {
            //function : "(!IN)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not(*(input_pin_type_to_bdd["IN"]));
            result["QN"]    = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_mux(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "MUX21X1" || gate->get_type() == "MUX21X2")
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) & bdd_not(*(input_pin_type_to_bdd["S"]));
            *bdd_output |= *(input_pin_type_to_bdd["IN2"]) & *(input_pin_type_to_bdd["S"]);
            result["Q"] = bdd_output;
        }
        if (gate->get_type() == "MUX41X1" || gate->get_type() == "MUX41X2")
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) & bdd_not(*(input_pin_type_to_bdd["S1"])) & bdd_not(*(input_pin_type_to_bdd["S0"]));
            *bdd_output |= *(input_pin_type_to_bdd["IN2"]) & bdd_not(*(input_pin_type_to_bdd["S1"])) & *(input_pin_type_to_bdd["S0"]);
            *bdd_output |= *(input_pin_type_to_bdd["IN3"]) & *(input_pin_type_to_bdd["S1"]) & bdd_not(*(input_pin_type_to_bdd["S0"]));
            *bdd_output |= *(input_pin_type_to_bdd["IN4"]) & *(input_pin_type_to_bdd["S1"]) & *(input_pin_type_to_bdd["S0"]);
            result["Q"] = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_dec(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "DEC24X1" || gate->get_type() == "DEC24X2")
        {
            //function : " ???";
            auto bdd_output_q0 = std::make_shared<bdd>();
            auto bdd_output_q1 = std::make_shared<bdd>();
            auto bdd_output_q2 = std::make_shared<bdd>();
            auto bdd_output_q3 = std::make_shared<bdd>();

            *bdd_output_q0 = bdd_not(*(input_pin_type_to_bdd["IN2"]) & *(input_pin_type_to_bdd["IN1"]));
            *bdd_output_q1 = bdd_not(*(input_pin_type_to_bdd["IN2"])) & *(input_pin_type_to_bdd["IN1"]);
            *bdd_output_q2 = bdd_not(*(input_pin_type_to_bdd["IN1"])) & *(input_pin_type_to_bdd["IN2"]);
            *bdd_output_q3 = *(input_pin_type_to_bdd["IN2"]) & *(input_pin_type_to_bdd["IN1"]);
            result["Q0"]   = bdd_output_q0;
            result["Q1"]   = bdd_output_q1;
            result["Q2"]   = bdd_output_q2;
            result["Q3"]   = bdd_output_q3;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_nand(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "NAND2X0" || gate->get_type() == "NAND2X1" || gate->get_type() == "NAND2X2")
        {
            //function : "(!(IN1 IN2))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not(*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"]));
            result["QN"]    = bdd_output;
        }
        if (gate->get_type() == "NAND3X0" || gate->get_type() == "NAND3X1" || gate->get_type() == "NAND3X2" || gate->get_type() == "NAND3X4")
        {
            //function : "(!((IN1 IN2) IN3))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not((*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) & *(input_pin_type_to_bdd["IN3"]));
            result["QN"]    = bdd_output;
        }
        if (gate->get_type() == "NAND4X0" || gate->get_type() == "NAND4X1")
        {
            //function : "(!(((IN1 IN2) IN3) IN4))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not(((*(input_pin_type_to_bdd["IN1"]) & *(input_pin_type_to_bdd["IN2"])) & *(input_pin_type_to_bdd["IN3"])) & *(input_pin_type_to_bdd["IN4"]));
            result["QN"]    = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_nor(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "NOR2X0" || gate->get_type() == "NOR2X1" || gate->get_type() == "NOR2X2" || gate->get_type() == "NOR2X4")
        {
            //function : "(!(IN1+IN2))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not(*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"]));
            result["QN"]    = bdd_output;
        }
        if (gate->get_type() == "NOR3X0" || gate->get_type() == "NOR3X1" || gate->get_type() == "NOR3X2" || gate->get_type() == "NOR3X4")
        {
            //function : "(!((IN1+IN2)+IN3))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not((*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) | *(input_pin_type_to_bdd["IN3"]));
            result["QN"]    = bdd_output;
        }
        if (gate->get_type() == "NOR4X0" || gate->get_type() == "NOR4X1")
        {
            //function : "(!(((IN1+IN2)+IN3)+IN4))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not(((*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) | *(input_pin_type_to_bdd["IN3"])) | *(input_pin_type_to_bdd["IN4"]));
            result["QN"]    = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_orandinv(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "OAI21X1" || gate->get_type() == "OAI21X2")
        {
            //function : "(!(IN1+IN2) IN3))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not((*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) & *(input_pin_type_to_bdd["IN3"]));
            result["QN"]    = bdd_output;
        }
        if (gate->get_type() == "OAI22X1" || gate->get_type() == "OAI22X2")
        {
            //function : "(!((IN1+IN") (IN3+IN4)))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not((*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) & (*(input_pin_type_to_bdd["IN3"]) | *(input_pin_type_to_bdd["IN4"])));
            result["QN"]    = bdd_output;
        }
        if (gate->get_type() == "OAI221X1" || gate->get_type() == "OAI221X2")
        {
            //function : "(!((IN1+IN") (IN3+IN4) IN5))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output =
                bdd_not(*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) & (*(input_pin_type_to_bdd["IN3"]) | *(input_pin_type_to_bdd["IN4"])) & *(input_pin_type_to_bdd["IN5"]);
            result["QN"] = bdd_output;
        }
        if (gate->get_type() == "OAI222X1" || gate->get_type() == "OAI222X2")
        {
            //function : "(!((IN1+IN") (IN3+IN4) (IN5+IN6)))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_not((*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) & (*(input_pin_type_to_bdd["IN3"]) | *(input_pin_type_to_bdd["IN4"]))
                                  & (*(input_pin_type_to_bdd["IN5"]) | *(input_pin_type_to_bdd["IN6"])));
            result["QN"]    = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_orand(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "OA21X1" || gate->get_type() == "OA21X2")
        {
            //function : "((IN1+IN2) IN3)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = (*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) & *(input_pin_type_to_bdd["IN3"]);
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "OA22X1" || gate->get_type() == "OA22X2")
        {
            //function : "((IN1+IN") (IN3+IN4))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = (*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) & (*(input_pin_type_to_bdd["IN3"]) | *(input_pin_type_to_bdd["IN4"]));
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "OA221X1" || gate->get_type() == "OA221X2")
        {
            //function : "((IN1+IN") (IN3+IN4) IN5)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output = (*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) & (*(input_pin_type_to_bdd["IN3"]) | *(input_pin_type_to_bdd["IN4"])) & *(input_pin_type_to_bdd["IN5"]);
            result["Q"] = bdd_output;
        }
        if (gate->get_type() == "OA222X1" || gate->get_type() == "OA222X2")
        {
            //function : "((IN1+IN") (IN3+IN4) (IN5+IN6))";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = (*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) & (*(input_pin_type_to_bdd["IN3"]) | *(input_pin_type_to_bdd["IN4"]))
                          & (*(input_pin_type_to_bdd["IN5"]) | *(input_pin_type_to_bdd["IN6"]));
            result["Q"] = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_or(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "OR2X1" || gate->get_type() == "OR2X2" || gate->get_type() == "OR2X4")
        {
            //function : "(IN1+IN2)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"]);
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "OR3X1" || gate->get_type() == "OR3X2" || gate->get_type() == "OR3X4")
        {
            //function : "(IN1+IN2+IN3)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"]) | *(input_pin_type_to_bdd["IN3"]);
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "OR4X1" || gate->get_type() == "OR4X2" || gate->get_type() == "OR4X4")
        {
            //function : "(((IN1+IN2)+IN3)+IN4)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = (((*(input_pin_type_to_bdd["IN1"]) | *(input_pin_type_to_bdd["IN2"])) | *(input_pin_type_to_bdd["IN3"])) | *(input_pin_type_to_bdd["IN4"]));
            result["Q"]     = bdd_output;
        }
        if (gate->get_type() == "ISOLORX1" || gate->get_type() == "ISOLORX2" || gate->get_type() == "ISOLORX4" || gate->get_type() == "ISOLORX8")
        {
            //function : "(D+ISO)";
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = *(input_pin_type_to_bdd["D"]) | *(input_pin_type_to_bdd["ISO"]);
            result["Q"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_hadd(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "HADDX1" || gate->get_type() == "HADDX2")
        {
            // pin(CO) function : "(A B)"  ;  pin(S) function : "(A^B)"
            auto bdd_output_c1 = std::make_shared<bdd>();
            *bdd_output_c1     = *(input_pin_type_to_bdd["A0"]) & *(input_pin_type_to_bdd["B0"]);
            auto bdd_output_s0 = std::make_shared<bdd>();
            *bdd_output_s0     = *(input_pin_type_to_bdd["A0"]) ^ *(input_pin_type_to_bdd["B0"]);
            result["C1"]       = bdd_output_c1;
            result["S0"]       = bdd_output_s0;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_fadd(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "FADDX1" || gate->get_type() == "FADDX2")
        {
            // pin(CO) function : "(((A B)+(B CI))+(CI A))"  ;  pin(S) function : "((A^B)^CI)"
            auto bdd_output_co = std::make_shared<bdd>();
            *bdd_output_co     = (((*(input_pin_type_to_bdd["A"]) & *(input_pin_type_to_bdd["B"])) | (*(input_pin_type_to_bdd["B"]) & *(input_pin_type_to_bdd["CI"])))
                              | (*(input_pin_type_to_bdd["CI"]) & *(input_pin_type_to_bdd["A"])));
            auto bdd_output_s  = std::make_shared<bdd>();
            *bdd_output_s      = ((*(input_pin_type_to_bdd["A"]) ^ *(input_pin_type_to_bdd["B"])) ^ *(input_pin_type_to_bdd["CI"]));
            result["CO"]       = bdd_output_co;
            result["S"]        = bdd_output_s;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_buf(std::shared_ptr<gate> gate, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if (gate->get_type() == "NBUFFX2")
        {
            auto o      = std::make_shared<bdd>();
            *o          = *(input_pin_type_to_bdd["IN"]);
            result["Q"] = o;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_vcc(std::shared_ptr<gate> gate, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if ((gate->get_type() == "GLOBAL_VCC"))
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_true();
            result["O"]     = bdd_output;
        }
        return result;
    }

    std::map<std::string, std::shared_ptr<bdd>> get_bdd_synopsys_gnd(std::shared_ptr<gate> gate, __attribute__((unused)) std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
    {
        std::map<std::string, std::shared_ptr<bdd>> result;
        if ((gate->get_type() == "GLOBAL_GND"))
        {
            auto bdd_output = std::make_shared<bdd>();
            *bdd_output     = bdd_false();
            result["O"]     = bdd_output;
        }
        return result;
    }
}    // namespace bdd_synopsys_helper

static std::map<std::string, gate_decorator_system::bdd_decorator_generator> m_bbd_generators = {
        {"INVX0", bdd_synopsys_helper::get_bdd_synopsys_inv},         {"INVX1", bdd_synopsys_helper::get_bdd_synopsys_inv},         {"INVX2", bdd_synopsys_helper::get_bdd_synopsys_inv},
        {"INVX4", bdd_synopsys_helper::get_bdd_synopsys_inv},         {"INVX8", bdd_synopsys_helper::get_bdd_synopsys_inv},         {"INVX16", bdd_synopsys_helper::get_bdd_synopsys_inv},
        {"INVX32", bdd_synopsys_helper::get_bdd_synopsys_inv},        {"IBUFFX2", bdd_synopsys_helper::get_bdd_synopsys_inv},       {"IBUFFX16", bdd_synopsys_helper::get_bdd_synopsys_inv},
        {"IBUFFX32", bdd_synopsys_helper::get_bdd_synopsys_inv},      {"AND2X1", bdd_synopsys_helper::get_bdd_synopsys_and},        {"AND2X2", bdd_synopsys_helper::get_bdd_synopsys_and},
        {"AND2X4", bdd_synopsys_helper::get_bdd_synopsys_and},        {"AND3X1", bdd_synopsys_helper::get_bdd_synopsys_and},        {"AND3X2", bdd_synopsys_helper::get_bdd_synopsys_and},
        {"AND3X4", bdd_synopsys_helper::get_bdd_synopsys_and},        {"AND4X1", bdd_synopsys_helper::get_bdd_synopsys_and},        {"AND4X2", bdd_synopsys_helper::get_bdd_synopsys_and},
        {"AND4X4", bdd_synopsys_helper::get_bdd_synopsys_and},        {"NAND2X0", bdd_synopsys_helper::get_bdd_synopsys_nand},      {"NAND2X1", bdd_synopsys_helper::get_bdd_synopsys_nand},
        {"NAND2X2", bdd_synopsys_helper::get_bdd_synopsys_nand},      {"NAND2X4", bdd_synopsys_helper::get_bdd_synopsys_nand},      {"NAND3X0", bdd_synopsys_helper::get_bdd_synopsys_nand},
        {"NAND3X1", bdd_synopsys_helper::get_bdd_synopsys_nand},      {"NAND3X2", bdd_synopsys_helper::get_bdd_synopsys_nand},      {"NAND3X4", bdd_synopsys_helper::get_bdd_synopsys_nand},
        {"NAND4X0", bdd_synopsys_helper::get_bdd_synopsys_nand},      {"NAND4X1", bdd_synopsys_helper::get_bdd_synopsys_nand},      {"OR2X1", bdd_synopsys_helper::get_bdd_synopsys_or},
        {"OR2X2", bdd_synopsys_helper::get_bdd_synopsys_or},          {"OR2X4", bdd_synopsys_helper::get_bdd_synopsys_or},          {"OR3X1", bdd_synopsys_helper::get_bdd_synopsys_or},
        {"OR3X2", bdd_synopsys_helper::get_bdd_synopsys_or},          {"OR3X4", bdd_synopsys_helper::get_bdd_synopsys_or},          {"OR4X1", bdd_synopsys_helper::get_bdd_synopsys_or},
        {"OR4X2", bdd_synopsys_helper::get_bdd_synopsys_or},          {"OR4X4", bdd_synopsys_helper::get_bdd_synopsys_or},          {"NOR2X0", bdd_synopsys_helper::get_bdd_synopsys_nor},
        {"NOR2X1", bdd_synopsys_helper::get_bdd_synopsys_nor},        {"NOR2X2", bdd_synopsys_helper::get_bdd_synopsys_nor},        {"NOR2X4", bdd_synopsys_helper::get_bdd_synopsys_nor},
        {"NOR3X0", bdd_synopsys_helper::get_bdd_synopsys_nor},        {"NOR3X1", bdd_synopsys_helper::get_bdd_synopsys_nor},        {"NOR3X2", bdd_synopsys_helper::get_bdd_synopsys_nor},
        {"NOR3X4", bdd_synopsys_helper::get_bdd_synopsys_nor},        {"NOR4X0", bdd_synopsys_helper::get_bdd_synopsys_nor},        {"NOR4X1", bdd_synopsys_helper::get_bdd_synopsys_nor},
        {"XOR2X1", bdd_synopsys_helper::get_bdd_synopsys_xor},        {"XOR2X2", bdd_synopsys_helper::get_bdd_synopsys_xor},        {"XOR3X1", bdd_synopsys_helper::get_bdd_synopsys_xor},
        {"XOR3X2", bdd_synopsys_helper::get_bdd_synopsys_xor},        {"XNOR2X1", bdd_synopsys_helper::get_bdd_synopsys_xnor},      {"XNOR2X2", bdd_synopsys_helper::get_bdd_synopsys_xnor},
        {"XNOR3X1", bdd_synopsys_helper::get_bdd_synopsys_xnor},      {"XNOR3X2", bdd_synopsys_helper::get_bdd_synopsys_xnor},      {"AO21X1", bdd_synopsys_helper::get_bdd_synopsys_andor},
        {"AO21X2", bdd_synopsys_helper::get_bdd_synopsys_andor},      {"AO22X1", bdd_synopsys_helper::get_bdd_synopsys_andor},      {"AO22X2", bdd_synopsys_helper::get_bdd_synopsys_andor},
        {"AO221X1", bdd_synopsys_helper::get_bdd_synopsys_andor},     {"AO221X2", bdd_synopsys_helper::get_bdd_synopsys_andor},     {"AO222X1", bdd_synopsys_helper::get_bdd_synopsys_andor},
        {"AO222X2", bdd_synopsys_helper::get_bdd_synopsys_andor},     {"AOI21X1", bdd_synopsys_helper::get_bdd_synopsys_andorinv},  {"AOI21X2", bdd_synopsys_helper::get_bdd_synopsys_andorinv},
        {"AOI22X1", bdd_synopsys_helper::get_bdd_synopsys_andorinv},  {"AOI22X2", bdd_synopsys_helper::get_bdd_synopsys_andorinv},  {"AOI221X1", bdd_synopsys_helper::get_bdd_synopsys_andorinv},
        {"AOI221X2", bdd_synopsys_helper::get_bdd_synopsys_andorinv}, {"AOI222X1", bdd_synopsys_helper::get_bdd_synopsys_andorinv}, {"AOI222X2", bdd_synopsys_helper::get_bdd_synopsys_andorinv},
        {"OA21X1", bdd_synopsys_helper::get_bdd_synopsys_orand},      {"OA21X2", bdd_synopsys_helper::get_bdd_synopsys_orand},      {"OA22X1", bdd_synopsys_helper::get_bdd_synopsys_orand},
        {"OA22X2", bdd_synopsys_helper::get_bdd_synopsys_orand},      {"OA221X1", bdd_synopsys_helper::get_bdd_synopsys_orand},     {"OA221X2", bdd_synopsys_helper::get_bdd_synopsys_orand},
        {"OA222X1", bdd_synopsys_helper::get_bdd_synopsys_orand},     {"OA222X2", bdd_synopsys_helper::get_bdd_synopsys_orand},     {"OAI21X1", bdd_synopsys_helper::get_bdd_synopsys_orandinv},
        {"OAI21X2", bdd_synopsys_helper::get_bdd_synopsys_orandinv},  {"OAI22X1", bdd_synopsys_helper::get_bdd_synopsys_orandinv},  {"OAI22X2", bdd_synopsys_helper::get_bdd_synopsys_orandinv},
        {"OAI221X1", bdd_synopsys_helper::get_bdd_synopsys_orandinv}, {"OAI221X2", bdd_synopsys_helper::get_bdd_synopsys_orandinv}, {"OAI222X1", bdd_synopsys_helper::get_bdd_synopsys_orandinv},
        {"OAI222X2", bdd_synopsys_helper::get_bdd_synopsys_orandinv}, {"MUX21X1", bdd_synopsys_helper::get_bdd_synopsys_mux},       {"MUX21X2", bdd_synopsys_helper::get_bdd_synopsys_mux},
        {"MUX41X1", bdd_synopsys_helper::get_bdd_synopsys_mux},       {"MUX41X2", bdd_synopsys_helper::get_bdd_synopsys_mux},       {"DEC24X1", bdd_synopsys_helper::get_bdd_synopsys_dec},
        {"DEC24X2", bdd_synopsys_helper::get_bdd_synopsys_dec},       {"HADDX1", bdd_synopsys_helper::get_bdd_synopsys_hadd},       {"HADDX2", bdd_synopsys_helper::get_bdd_synopsys_hadd},
        {"FADDX1", bdd_synopsys_helper::get_bdd_synopsys_fadd},       {"FADDX2", bdd_synopsys_helper::get_bdd_synopsys_fadd},       {"ISOLANDX1", bdd_synopsys_helper::get_bdd_synopsys_and},
        {"ISOLANDX2", bdd_synopsys_helper::get_bdd_synopsys_and},     {"ISOLANDX4", bdd_synopsys_helper::get_bdd_synopsys_and},     {"ISOLANDX8", bdd_synopsys_helper::get_bdd_synopsys_and},
        {"ISOLORX1", bdd_synopsys_helper::get_bdd_synopsys_or},       {"ISOLORX2", bdd_synopsys_helper::get_bdd_synopsys_or},       {"ISOLORX4", bdd_synopsys_helper::get_bdd_synopsys_or},
        {"ISOLORX8", bdd_synopsys_helper::get_bdd_synopsys_or},       {"GLOBAL_VCC", bdd_synopsys_helper::get_bdd_synopsys_vcc},    {"GLOBAL_GND", bdd_synopsys_helper::get_bdd_synopsys_gnd},
        {"ISOLANDX1", bdd_synopsys_helper::get_bdd_synopsys_and},     {"NBUFFX2", bdd_synopsys_helper::get_bdd_synopsys_buf},
};

std::map<std::string, std::shared_ptr<bdd>> plugin_gate_decorators::bdd_generator_synopsys90(std::shared_ptr<gate> g, std::map<std::string, std::shared_ptr<bdd>>& input_pin_type_to_bdd)
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

bool plugin_gate_decorators::bdd_availability_tester_synopsys90(std::shared_ptr<gate> g)
{
    auto type = g->get_type();
    if (m_bbd_generators.find(type) == m_bbd_generators.end())
    {
        return false;
    }
    return true;
}
