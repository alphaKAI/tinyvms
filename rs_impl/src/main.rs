use std::vec;
extern crate num;
use num::traits::FromPrimitive;

type SerializedType = i64;

#[derive(Clone, Debug)]
struct Env {}

#[derive(Clone, Debug)]
struct VMFunction {
    func_name: String,
    func_body: Vec<Opcode>,
    env: Env,
}

#[derive(Clone, Debug)]
enum Value {
    Long(Option<i64>),
    String(Option<String>),
    Bool(Option<bool>),
    Array(Option<Vec<Value>>),
    Function(Option<VMFunction>),
    Null,
}

impl FromPrimitive for Value {
    fn from_i64(n: i64) -> Option<Value> {
        match n {
            0 => Some(Value::Long(None)),
            1 => Some(Value::String(None)),
            2 => Some(Value::Bool(None)),
            3 => Some(Value::Array(None)),
            4 => Some(Value::Function(None)),
            5 => Some(Value::Null),
            _ => None,
        }
    }

    fn from_u64(n: u64) -> Option<Value> {
        match n {
            0 => Some(Value::Long(None)),
            1 => Some(Value::String(None)),
            2 => Some(Value::Bool(None)),
            3 => Some(Value::Array(None)),
            4 => Some(Value::Function(None)),
            5 => Some(Value::Null),
            _ => None,
        }
    }
}

#[derive(Debug, Clone)]
enum Opcode {
    OpVariableDeclareOnlySymbol(Option<String>),
    OpVariableDeclareWithAssign(Option<String>),
    OpPop,
    OpPush(Option<Value>),
    OpAdd,
    OpSub,
    OpMul,
    OpDiv,
    OpMod,
    OpReturn,
    OpGetVariable(Option<String>),
    OpSetVariablePop(Option<String>),
    OpSetArrayElement(Option<String>),
    OpGetArrayElement(Option<String>),
    OpMakeArray(Option<i64>),
    OpCall(Option<String>),
    OpNop,
    OpFunctionDeclare(Option<(String, i64)>),
    OpEqualExpression,
    OpNotEqualExpression,
    OpLtExpression,
    OpLteExpression,
    OpGtExpression,
    OpGteExpression,
    OpAndExpression,
    OpOrExpression,
    OpXorExpression,
    OpJumpRel(Option<i64>),
    OpJumpAbs(Option<i64>),
    OpPrint,
    OpPrintln,
    OpIFStatement(Option<i64>),
    OpAssignExpression(Option<String>),
    OpAssert,
    IValue,
}

impl Opcode {
    fn opsize(op: Opcode) -> i64 {
        match op {
            Opcode::OpVariableDeclareOnlySymbol(_) | Opcode::OpVariableDeclareWithAssign(_) => 2,
            Opcode::OpPop => 1,
            Opcode::OpPush(_) => 2,
            Opcode::OpAdd
            | Opcode::OpSub
            | Opcode::OpMul
            | Opcode::OpDiv
            | Opcode::OpMod
            | Opcode::OpReturn => 1,
            Opcode::OpGetVariable(_)
            | Opcode::OpSetVariablePop(_)
            | Opcode::OpSetArrayElement(_)
            | Opcode::OpGetArrayElement(_)
            | Opcode::OpMakeArray(_)
            | Opcode::OpCall(_) => 2,
            Opcode::OpNop => 1,
            Opcode::OpFunctionDeclare(_) => 3,
            Opcode::OpEqualExpression
            | Opcode::OpNotEqualExpression
            | Opcode::OpLtExpression
            | Opcode::OpLteExpression
            | Opcode::OpGtExpression
            | Opcode::OpGteExpression
            | Opcode::OpAndExpression
            | Opcode::OpOrExpression
            | Opcode::OpXorExpression => 1,
            Opcode::OpJumpRel(_) | Opcode::OpJumpAbs(_) => 2,
            Opcode::OpPrint => 1,
            Opcode::OpPrintln => 1,
            Opcode::OpIFStatement(_) | Opcode::OpAssignExpression(_) => 2,
            Opcode::OpAssert => 1,
            Opcode::IValue => panic!("Unsupported"),
        }
    }
}

impl FromPrimitive for Opcode {
    fn from_i64(n: i64) -> Option<Opcode> {
        match n {
            0 => Some(Opcode::OpVariableDeclareOnlySymbol(None)),
            1 => Some(Opcode::OpVariableDeclareWithAssign(None)),
            2 => Some(Opcode::OpPop),
            3 => Some(Opcode::OpPush(None)),
            4 => Some(Opcode::OpAdd),
            5 => Some(Opcode::OpSub),
            6 => Some(Opcode::OpMul),
            7 => Some(Opcode::OpDiv),
            8 => Some(Opcode::OpMod),
            9 => Some(Opcode::OpReturn),
            10 => Some(Opcode::OpGetVariable(None)),
            11 => Some(Opcode::OpSetVariablePop(None)),
            12 => Some(Opcode::OpSetArrayElement(None)),
            13 => Some(Opcode::OpGetArrayElement(None)),
            14 => Some(Opcode::OpMakeArray(None)),
            15 => Some(Opcode::OpCall(None)),
            16 => Some(Opcode::OpNop),
            17 => Some(Opcode::OpFunctionDeclare(None)),
            18 => Some(Opcode::OpEqualExpression),
            19 => Some(Opcode::OpNotEqualExpression),
            20 => Some(Opcode::OpLtExpression),
            21 => Some(Opcode::OpLteExpression),
            22 => Some(Opcode::OpGtExpression),
            23 => Some(Opcode::OpGteExpression),
            24 => Some(Opcode::OpAndExpression),
            25 => Some(Opcode::OpOrExpression),
            26 => Some(Opcode::OpXorExpression),
            27 => Some(Opcode::OpJumpRel(None)),
            28 => Some(Opcode::OpJumpAbs(None)),
            29 => Some(Opcode::OpPrint),
            30 => Some(Opcode::OpPrintln),
            31 => Some(Opcode::OpIFStatement(None)),
            32 => Some(Opcode::OpAssignExpression(None)),
            33 => Some(Opcode::OpAssert),
            34 => Some(Opcode::IValue),
            _ => None,
        }
    }

    fn from_u64(n: u64) -> Option<Opcode> {
        Opcode::from_i64(n as i64)
    }
}

struct TValueDeserializeResult {
    pub array: Vec<Value>,
    pub idx: usize,
}

impl TValueDeserializeResult {
    pub fn new(array: Vec<Value>, idx: usize) -> TValueDeserializeResult {
        TValueDeserializeResult {
            array: array,
            idx: idx,
        }
    }
}

fn int_to_bool(t: i64) -> bool {
    match t {
        0 => true,
        1 => false,
        _ => panic!("can't convert non 0 or 1 value into bool"),
    }
}

fn deserialize_to_tva(
    serialized: &Vec<SerializedType>,
    elem_size: usize,
) -> TValueDeserializeResult {
    let mut array = vec![];
    let mut idx = 0;

    while idx < serialized.len() && idx < elem_size {
        let ty = serialized[idx];
        idx += 1;

        match Value::from_i64(ty as i64) {
            Some(v) => {
                match v {
                    Value::Long(_) => {
                        array.push(Value::Long(Some(serialized[idx])));
                        idx += 1;
                    }
                    Value::String(_) => {
                        let len = serialized[idx];
                        idx += 1;
                        let mut v = vec![];

                        for _ in 0..len {
                            v.push(std::char::from_u32(serialized[idx] as u32).unwrap());
                            idx += 1;
                        }

                        let s = v.into_iter().collect();
                        array.push(Value::String(Some(s)));
                    }
                    Value::Bool(_) => {
                        array.push(Value::Bool(Some(int_to_bool(serialized[idx]))));
                        idx += 1;
                    }
                    Value::Array(_) => {
                        let len = serialized[idx];
                        idx += 1;
                        let mut ve = vec![];
                        let mut i = 0;
                        while i < serialized.len() {
                            ve.push(serialized[i]);
                            i += 1;
                        }

                        let tvdr = deserialize_to_tva(&ve, len as usize);
                        let arr = tvdr.array;
                        idx += tvdr.idx;
                        array.push(Value::Array(Some(arr)));
                    },
                    _ => panic!("unsupported ty: {:?}", ty),
                }
            }
            None => panic!("Unkown ty: {:?}", ty),
        };
    }

    return TValueDeserializeResult::new(array, idx);
}

fn proc_with_1arg(
    dst: &mut Vec<Opcode>,
    serialized: &Vec<SerializedType>,
    ty: i64,
    idx: &mut usize,
) {
    let mut v = vec![];

    let mut i = *idx;
    while i < serialized.len() {
        v.push(serialized[i]);
        i += 1;
    }

    let tvdr = deserialize_to_tva(&v, 1);
    //println!("tvdr.array: {:?}", tvdr.array);
    let t = match ty {
        0 => match tvdr.array[0].clone() {
            Value::String(s_opt) => Opcode::OpVariableDeclareOnlySymbol(s_opt),
            _ => panic!("string expected"),
        },
        1 => match tvdr.array[0].clone() {
            Value::String(s_opt) => Opcode::OpVariableDeclareWithAssign(s_opt),
            _ => panic!("string expected"),
        },
        2 => Opcode::OpPop,
        3 => Opcode::OpPush(Some(tvdr.array[0].clone())),
        10 => match tvdr.array[0].clone() {
            Value::String(s_opt) => Opcode::OpGetVariable(s_opt),
            _ => panic!("string expected"),
        },
        11 => match tvdr.array[0].clone() {
            Value::String(s_opt) => Opcode::OpSetVariablePop(s_opt),
            _ => panic!("string expected"),
        },
        12 => match tvdr.array[0].clone() {
            Value::String(s_opt) => Opcode::OpSetArrayElement(s_opt),
            _ => panic!("string expected"),
        },
        13 => match tvdr.array[0].clone() {
            Value::String(s_opt) => Opcode::OpGetArrayElement(s_opt),
            _ => panic!("string expected"),
        },
        14 => match tvdr.array[0].clone() {
            Value::Long(l_opt) => Opcode::OpMakeArray(l_opt),
            _ => panic!("integer expected"),
        },
        15 => match tvdr.array[0].clone() {
            Value::String(s_opt) => Opcode::OpCall(s_opt),
            _ => panic!("string expected"),
        },
        27 => match tvdr.array[0].clone() {
            Value::Long(l_opt) => Opcode::OpJumpRel(l_opt),
            _ => panic!("integer expected"),
        },
        28 => match tvdr.array[0].clone() {
            Value::Long(l_opt) => Opcode::OpJumpAbs(l_opt),
            _ => panic!("integer expected"),
        },
        31 => match tvdr.array[0].clone() {
            Value::Long(l_opt) => Opcode::OpIFStatement(l_opt),
            _ => panic!("integer expected"),
        },
        32 => match tvdr.array[0].clone() {
            Value::String(l_opt) => Opcode::OpAssignExpression(l_opt),
            _ => panic!("string expected"),
        },
        _ => panic!("unsupported ty: {:?}", ty),
    };
    dst.push(t);
    *idx += tvdr.idx;
}

fn proc_with_2arg(
    dst: &mut Vec<Opcode>,
    serialized: &Vec<SerializedType>,
    ty: i64,
    idx: &mut usize,
) {
    let mut v = vec![];
    let mut i = *idx;
    while i < serialized.len() {
        v.push(serialized[i]);
        i += 1;
    }
    let tvdr1 = deserialize_to_tva(&v, 1);
    //println!("tvdr1.array: {:?}", tvdr1.array);
    *idx += tvdr1.idx;

    i = *idx;
    v = vec![];
    while i < serialized.len() {
        v.push(serialized[i]);
        i += 1;
    }
    let tvdr2 = deserialize_to_tva(&v, 1);
    //println!("tvdr2.array: {:?}", tvdr2.array);
    *idx += tvdr2.idx;

    let t = match ty {
        17 => match (tvdr1.array[0].clone(), tvdr2.array[0].clone()) {
            (Value::String(Some(s)), Value::Long(Some(l))) => {
                Opcode::OpFunctionDeclare(Some((s, l)))
            }
            _ => panic!("Invalid format"),
        },
        _ => panic!("unsupported"),
    };
    dst.push(t);
}

fn deserialize(serialized: Vec<SerializedType>) -> Vec<Opcode> {
    let mut idx = 0;
    let mut ret = vec![];

    while idx < serialized.len() {
        let ty = serialized[idx];
        idx += 1;

        match Opcode::from_i64(ty) {
            Some(op) => match op {
                Opcode::OpVariableDeclareOnlySymbol(_) | Opcode::OpVariableDeclareWithAssign(_) => {
                    proc_with_1arg(&mut ret, &serialized, ty, &mut idx)
                }
                Opcode::OpPop => panic!("<Deserialize> Not supported {:?}", ty),
                Opcode::OpPush(_) => proc_with_1arg(&mut ret, &serialized, ty, &mut idx),
                Opcode::OpAdd
                | Opcode::OpSub
                | Opcode::OpMul
                | Opcode::OpMod
                | Opcode::OpReturn => ret.push(op),
                Opcode::OpGetVariable(_)
                | Opcode::OpSetVariablePop(_)
                | Opcode::OpSetArrayElement(_)
                | Opcode::OpGetArrayElement(_)
                | Opcode::OpMakeArray(_)
                | Opcode::OpCall(_) => proc_with_1arg(&mut ret, &serialized, ty, &mut idx),
                Opcode::OpNop => {
                    ret.push(op);
                }
                Opcode::OpFunctionDeclare(_) => proc_with_2arg(&mut ret, &serialized, ty, &mut idx),
                Opcode::OpEqualExpression
                | Opcode::OpNotEqualExpression
                | Opcode::OpLtExpression
                | Opcode::OpLteExpression
                | Opcode::OpGtExpression
                | Opcode::OpGteExpression
                | Opcode::OpAndExpression
                | Opcode::OpOrExpression
                | Opcode::OpXorExpression => ret.push(op),
                Opcode::OpJumpRel(_) | Opcode::OpJumpAbs(_) => proc_with_1arg(&mut ret, &serialized, ty, &mut idx),
                Opcode::OpPrint | Opcode::OpPrintln => ret.push(op),
                Opcode::OpIFStatement(_) | Opcode::OpAssignExpression(_) => proc_with_1arg(&mut ret, &serialized, ty, &mut idx),
                Opcode::OpAssert => ret.push(op),
                _ => panic!("unsupported! ty: {:?}", ty),
            },
            None => panic!("Unknown! ty: {:?}", ty),
        }
    }

    return ret;
}

use std::io::{BufReader, Read};
use std::{fs, mem};

fn read_from_file(filename: String) -> Vec<SerializedType> {
    let fh = fs::File::open(filename).unwrap();
    let mut f = BufReader::new(fh);
    let mut buf: [u8; 8] = unsafe { mem::uninitialized() };
    let mut ret = vec![];

    while let Ok(_) = f.read_exact(&mut buf) {
        let mut b: i64 = 0;
        for i in 0..8 {
            b |= (buf[i] as i64) << (i * 8);
        }
        ret.push(b);
    }

    return ret;
}

fn main() {
    let v = vec![
        "samples/test_1.toy.compiled",
        "samples/test_2.toy.compiled",
        "samples/test_3.toy.compiled",
        "samples/test_4.toy.compiled",
        "samples/test_5.toy.compiled",
        "samples/test_6.toy.compiled",
        "samples/test_7.toy.compiled",
        "samples/test_8.toy.compiled",
        "samples/test_9.toy.compiled",
        "samples/test_10.toy.compiled",
        "samples/test_11.toy.compiled",
        "samples/test_12.toy.compiled",
        "samples/test_12.toy.compiled",
        "samples/test_13.toy.compiled",
        "samples/test_14.toy.compiled",
        "samples/test_15.toy.compiled",
        "samples/test_16.toy.compiled",
    ];

    for p in v.into_iter() {
        println!("[test] - {:?}", p);
        let input = read_from_file(p.to_string());
        println!("\tByte Code     : {:?}", input);
        let x = deserialize(input);
        println!("\tInstructions : {:?}", x);
    }
}
