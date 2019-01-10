contract Base {
    function f() public pure returns (uint) {}
}
contract Test {
    function creation() public pure returns (bytes memory) {
        return type(Test).creationCode;
    }
    function runtime() public pure returns (bytes memory) {
        return type(Test).runtimeCode;
    }
    function creationBase() public pure returns (bytes memory) {
        return type(Base).creationCode;
    }
    function runtimeBase() public pure returns (bytes memory) {
        return type(Base).runtimeCode;
    }
}
// ----