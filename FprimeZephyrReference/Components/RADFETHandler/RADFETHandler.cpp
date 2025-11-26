// ======================================================================
// \title  RADFETHandler.cpp
// \author kai
// \brief  cpp file for RADFETHandler component implementation class
// ======================================================================

#include "FprimeZephyrReference/Components/RADFETHandler/RADFETHandler.hpp"
#include <Fw/Types/Assert.hpp>
#include <Os/File.hpp>

namespace FprimeZephyrReference {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

RADFETHandler ::RADFETHandler(const char* const compName) : RADFETHandlerComponentBase(compName) {}

RADFETHandler ::~RADFETHandler() {}

}  // namespace FprimeZephyrReference
