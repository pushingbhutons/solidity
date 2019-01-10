contract Test {
    function creation() public pure returns (bytes memory) {
        return type(Test).creationCode;
    }
    function runtime() public pure returns (bytes memory) {
        return type(Test).runtimeCode;
    }
    function creationOther() public pure returns (bytes memory) {
        return type(Other).creationCode;
    }
    function runtimeOther() public pure returns (bytes memory) {
        return type(Other).runtimeCode;
    }
}
contract Other {
    function f(uint) public pure returns (uint) {}
}
// ----