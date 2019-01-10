contract Test {
    bytes constant c = type(Test).creationCode;
    bytes constant r = type(Test).runtimeCode;

}
// ----
