

#include "OpcodeMapper.h"
#include "Opcode.h"
#include "evmlir/Dialect/EVM/EVMOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/Operation.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/TypeSwitch.h"
#include <optional>

std::optional<Opcode> getOpcode(mlir::Operation *op) {
  return llvm::TypeSwitch<mlir::Operation *, std::optional<Opcode>>(op)
      .Case<evmlir::evm::InlineBytecodeOp>([](auto) { return std::nullopt; })
      .Case<evmlir::evm::StopOp>([](auto) { return Opcode::STOP; })
      .Case<mlir::arith::AddIOp>([](auto) { return Opcode::ADD; })
      .Case<mlir::arith::MulIOp>([](auto) { return Opcode::MUL; })
      .Case<mlir::arith::SubIOp>([](auto) { return Opcode::SUB; })
      .Case<mlir::arith::DivUIOp>([](auto) { return Opcode::DIV; })
      .Case<mlir::arith::DivSIOp>([](auto) { return Opcode::SDIV; })
      .Case<mlir::arith::RemUIOp>([](auto) { return Opcode::MOD; })
      .Case<mlir::arith::RemSIOp>([](auto) { return Opcode::SMOD; })
      .Case<evmlir::evm::AddmodOp>([](auto) { return Opcode::ADDMOD; })
      .Case<evmlir::evm::MulmodOp>([](auto) { return Opcode::MULMOD; })
      .Case<evmlir::evm::ExpOp>([](auto) { return Opcode::EXP; })
      .Case<evmlir::evm::SignextendOp>(
          [](auto) { return Opcode::SIGNEXTEND; })
      .Case<mlir::arith::CmpIOp>([](auto op) -> std::optional<Opcode> {
        switch (op.getPredicate()) {
        case mlir::arith::CmpIPredicate::ult:
          return Opcode::LT;
        case mlir::arith::CmpIPredicate::ugt:
          return Opcode::GT;
        case mlir::arith::CmpIPredicate::slt:
          return Opcode::SLT;
        case mlir::arith::CmpIPredicate::sgt:
          return Opcode::SGT;
        case mlir::arith::CmpIPredicate::eq: {
          auto isZero = [](mlir::Value v) -> bool {
            auto cst = v.getDefiningOp<mlir::arith::ConstantOp>();
            if (!cst)
              return false;
            auto intAttr = mlir::dyn_cast<mlir::IntegerAttr>(cst.getValue());
            if (!intAttr)
              return false;
            return intAttr.getValue().isZero();
          };

          bool lhsZero = isZero(op.getLhs());
          bool rhsZero = isZero(op.getRhs());
          if (lhsZero != rhsZero)
            return Opcode::ISZERO;
          return Opcode::EQ;
        }
        default:
          return std::nullopt;
        }
      })
      .Case<mlir::arith::AndIOp>([](auto) { return Opcode::AND; })
      .Case<mlir::arith::OrIOp>([](auto) { return Opcode::OR; })
      .Case<mlir::arith::XOrIOp>([](auto op) -> std::optional<Opcode> {
        auto isAllOnes = [](mlir::Value v) -> bool {
          auto cst = v.getDefiningOp<mlir::arith::ConstantOp>();
          if (!cst)
            return false;
          auto intAttr = mlir::dyn_cast<mlir::IntegerAttr>(cst.getValue());
          if (!intAttr)
            return false;
          return intAttr.getValue().isAllOnes();
        };

        if (isAllOnes(op.getLhs()) || isAllOnes(op.getRhs()))
          return Opcode::NOT;
        return Opcode::XOR;
      })
      .Case<evmlir::evm::ByteOp>([](auto) { return Opcode::BYTE; })
      .Case<mlir::arith::ShLIOp>([](auto) { return Opcode::SHL; })
      .Case<mlir::arith::ShRUIOp>([](auto) { return Opcode::SHR; })
      .Case<mlir::arith::ShRSIOp>([](auto) { return Opcode::SAR; })
      .Case<mlir::math::CountLeadingZerosOp>([](auto) { return Opcode::CLZ; })
      .Case<evmlir::evm::Keccak256Op>([](auto) { return Opcode::KECCAK256; })
      .Case<evmlir::evm::AddressOp>([](auto) { return Opcode::ADDRESS; })
      .Case<evmlir::evm::BalanceOp>([](auto) { return Opcode::BALANCE; })
      .Case<evmlir::evm::OriginOp>([](auto) { return Opcode::ORIGIN; })
      .Case<evmlir::evm::CallerOp>([](auto) { return Opcode::CALLER; })
      .Case<evmlir::evm::CallvalueOp>([](auto) { return Opcode::CALLVALUE; })
      .Case<evmlir::evm::CalldataloadOp>(
          [](auto) { return Opcode::CALLDATALOAD; })
      .Case<evmlir::evm::CalldatasizeOp>(
          [](auto) { return Opcode::CALLDATASIZE; })
      .Case<evmlir::evm::CalldatacopyOp>(
          [](auto) { return Opcode::CALLDATACOPY; })
      .Case<evmlir::evm::CodesizeOp>([](auto) { return Opcode::CODESIZE; })
      .Case<evmlir::evm::CodecopyOp>([](auto) { return Opcode::CODECOPY; })
      .Case<evmlir::evm::GaspriceOp>([](auto) { return Opcode::GASPRICE; })
      .Case<evmlir::evm::ExtcodesizeOp>(
          [](auto) { return Opcode::EXTCODESIZE; })
      .Case<evmlir::evm::ExtcodecopyOp>(
          [](auto) { return Opcode::EXTCODECOPY; })
      .Case<evmlir::evm::ReturndatasizeOp>(
          [](auto) { return Opcode::RETURNDATASIZE; })
      .Case<evmlir::evm::ReturndatacopyOp>(
          [](auto) { return Opcode::RETURNDATACOPY; })
      .Case<evmlir::evm::ExtcodehashOp>(
          [](auto) { return Opcode::EXTCODEHASH; })
      .Case<evmlir::evm::BlockhashOp>([](auto) { return Opcode::BLOCKHASH; })
      .Case<evmlir::evm::CoinbaseOp>([](auto) { return Opcode::COINBASE; })
      .Case<evmlir::evm::TimestampOp>([](auto) { return Opcode::TIMESTAMP; })
      .Case<evmlir::evm::NumberOp>([](auto) { return Opcode::NUMBER; })
      .Case<evmlir::evm::PrevrandaoOp>(
          [](auto) { return Opcode::PREVRANDAO; })
      .Case<evmlir::evm::GaslimitOp>([](auto) { return Opcode::GASLIMIT; })
      .Case<evmlir::evm::ChainidOp>([](auto) { return Opcode::CHAINID; })
      .Case<evmlir::evm::SelfbalanceOp>(
          [](auto) { return Opcode::SELFBALANCE; })
      .Case<evmlir::evm::BasefeeOp>([](auto) { return Opcode::BASEFEE; })
      .Case<evmlir::evm::BlobhashOp>([](auto) { return Opcode::BLOBHASH; })
      .Case<evmlir::evm::BlobbasefeeOp>(
          [](auto) { return Opcode::BLOBBASEFEE; })

      .Case<evmlir::evm::MloadOp>([](auto) { return Opcode::MLOAD; })
      .Case<evmlir::evm::MstoreOp>([](auto) { return Opcode::MSTORE; })
      .Case<evmlir::evm::Mstore8Op>([](auto) { return Opcode::MSTORE8; })
      .Case<evmlir::evm::SloadOp>([](auto) { return Opcode::SLOAD; })
      .Case<evmlir::evm::SstoreOp>([](auto) { return Opcode::SSTORE; })

      .Case<evmlir::evm::PcOp>([](auto) { return Opcode::PC; })
      .Case<evmlir::evm::MsizeOp>([](auto) { return Opcode::MSIZE; })
      .Case<evmlir::evm::GasOp>([](auto) { return Opcode::GAS; })
      .Case<evmlir::evm::TloadOp>([](auto) { return Opcode::TLOAD; })
      .Case<evmlir::evm::TstoreOp>([](auto) { return Opcode::TSTORE; })
      .Case<evmlir::evm::McopyOp>([](auto) { return Opcode::MCOPY; })

      .Case<evmlir::evm::Log0Op>([](auto) { return Opcode::LOG0; })
      .Case<evmlir::evm::Log1Op>([](auto) { return Opcode::LOG1; })
      .Case<evmlir::evm::Log2Op>([](auto) { return Opcode::LOG2; })
      .Case<evmlir::evm::Log3Op>([](auto) { return Opcode::LOG3; })
      .Case<evmlir::evm::Log4Op>([](auto) { return Opcode::LOG4; })

      .Case<evmlir::evm::CreateOp>([](auto) { return Opcode::CREATE; })
      .Case<evmlir::evm::CallOp>([](auto) { return Opcode::CALL; })
      .Case<evmlir::evm::CallcodeOp>([](auto) { return Opcode::CALLCODE; })
      .Case<evmlir::evm::ReturnOp>([](auto) { return Opcode::RETURN; })

      .Case<evmlir::evm::DelegatecallOp>(
          [](auto) { return Opcode::DELEGATECALL; })
      .Case<evmlir::evm::Create2Op>([](auto) { return Opcode::CREATE2; })
      .Case<evmlir::evm::StaticcallOp>(
          [](auto) { return Opcode::STATICCALL; })
      .Case<evmlir::evm::RevertOp>([](auto) { return Opcode::REVERT; })
      .Case<evmlir::evm::InvalidOp>([](auto) { return Opcode::INVALID; })
      .Case<evmlir::evm::SelfdestructOp>(
          [](auto) { return Opcode::SELFDESTRUCT; })
      .Default([](auto) { return std::nullopt; });
}