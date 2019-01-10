contract Test {
    struct S { uint x; }
    function creation() public pure {
        // Unsupported for now, but might be supported in the future
        type(S);
    }
}
// ----
// TypeError: (161-162): Invalid type for argument in function call. Contract type required, but type(struct Test.S) provided.
