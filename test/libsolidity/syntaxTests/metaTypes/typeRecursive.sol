contract Test {
    function creation() public pure {
        type(type(type(Test)));
    }
}
// ----
// TypeError: (72-82): Invalid type for argument in function call. Contract type required, but type(contract Test) provided.
// TypeError: (67-83): Invalid type for argument in function call. Contract type required, but tuple() provided.
