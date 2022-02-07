module Ref {
    @ Port for requesting an operation on two numbers
    port MathOp(
        val1: F32 @< the first operand
        op: MathOp @< the operation
        val2: F32 @< the second operand
    )

    @ Port for returning the result of a math operation
    port MathResult(
        result: F32 @< the result of the operation
    )
}