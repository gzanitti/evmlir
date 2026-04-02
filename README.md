# evmlir
 
An out-of-tree [MLIR](https://mlir.llvm.org/) compiler infrastructure targeting the
[Ethereum Virtual Machine](https://ethereum.org/en/developers/docs/evm/) (EVM).
 
> Work in progress. The dialect design is stable; the translation layer and backend are
> under active development.
 
## What this is
 
`evmlir` defines a single MLIR dialect (`evm`) that models EVM-specific constructs —
storage, memory, calldata, external calls, logging, contract lifecycle — as first-class
MLIR operations. Everything that standard dialects can express (`arith`, `cf`, `scf`,
`func`, `math`) stays there. The `evm` dialect covers only what they cannot.
 
The compilation pipeline looks like this:
 
```
arith + cf + scf + func + math + evm
              ↓  translation (stack scheduling + emit)
         EVM bytecode
```