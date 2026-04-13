use revm::{
    db::InMemoryDB,
    primitives::{Address, Bytes, ExecutionResult, TxKind, U256},
    Evm,
};

pub struct RevmState {
    db: InMemoryDB,
    deployed_address: Option<Address>,
}

#[no_mangle]
pub extern "C" fn revm_new() -> *mut RevmState {
    Box::into_raw(Box::new(RevmState {
        db: InMemoryDB::default(),
        deployed_address: None,
    }))
}

#[no_mangle]
pub extern "C" fn revm_deploy(
    state: *mut RevmState,
    bytecode: *const u8,
    bytecode_len: usize,
) -> bool {
    let state = unsafe {
        if state.is_null() { return false; }
        &mut *state
    };
    let deploy_bytes = unsafe { std::slice::from_raw_parts(bytecode, bytecode_len) };

    let mut evm = Evm::builder()
        .with_db(&mut state.db)
        .modify_tx_env(|tx| {
            tx.caller = Address::ZERO;
            tx.transact_to = TxKind::Create;
            tx.data = Bytes::copy_from_slice(deploy_bytes);
            tx.value = U256::ZERO;
            tx.gas_limit = 30_000_000;
            tx.gas_price = U256::ZERO;
        })
        .build();

    match evm.transact_commit() {
        Ok(ExecutionResult::Success {
            output: revm::primitives::Output::Create(_, Some(addr)),
            ..
        }) => {
            state.deployed_address = Some(addr);
            true
        }
        _ => false,
    }
}

#[no_mangle]
pub extern "C" fn revm_call(
    state: *mut RevmState,
    calldata: *const u8,
    calldata_len: usize,
    out_data: *mut u8,
    out_data_cap: usize,
    out_data_len: *mut usize,
) -> bool {
    let state = unsafe {
        if state.is_null() { return false; }
        &mut *state
    };
    let addr = match state.deployed_address {
        Some(a) => a,
        None => return false,
    };
    let call_bytes = unsafe { std::slice::from_raw_parts(calldata, calldata_len) };

    let mut evm = Evm::builder()
        .with_db(&mut state.db)
        .modify_tx_env(|tx| {
            tx.caller = Address::ZERO;
            tx.transact_to = TxKind::Call(addr);
            tx.data = Bytes::copy_from_slice(call_bytes);
            tx.value = U256::ZERO;
            tx.gas_limit = 30_000_000;
            tx.gas_price = U256::ZERO;
        })
        .build();

    match evm.transact_commit() {
        Ok(ExecutionResult::Success { output, .. }) => {
            let return_data: &[u8] = match &output {
                revm::primitives::Output::Call(bytes) => bytes,
                revm::primitives::Output::Create(bytes, _) => bytes,
            };
            let copy_len = return_data.len().min(out_data_cap);
            unsafe {
                std::ptr::copy_nonoverlapping(return_data.as_ptr(), out_data, copy_len);
                *out_data_len = return_data.len();
            }
            true
        }
        _ => false,
    }
}

#[no_mangle]
pub extern "C" fn revm_free(state: *mut RevmState) {
    if !state.is_null() {
        unsafe { drop(Box::from_raw(state)) };
    }
}
