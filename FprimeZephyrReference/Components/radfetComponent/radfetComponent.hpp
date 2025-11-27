// ======================================================================
// \title  radfetComponent.hpp
// \author kai
// \brief  hpp file for radfetComponent component implementation class
// ======================================================================

#ifndef Components_radfetComponent_HPP
#define Components_radfetComponent_HPP

#include "FprimeZephyrReference/Components/radfetComponent/radfetComponentComponentAc.hpp"

namespace Components {

class radfetComponent final : public radfetComponentComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct radfetComponent object
    radfetComponent(const char* const compName  //!< The component name
    );

    //! Destroy radfetComponent object
    ~radfetComponent();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command TODO
    //!
    //! TODO
    //void TODO_cmdHandler(FwOpcodeType opCode,  //!< The opcode
    //                     U32 cmdSeq            //!< The command sequence number
    //                     ) override;
};

}  // namespace Components

#endif
