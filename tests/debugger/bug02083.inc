<?php

enum TestEnum {
    case EnumCase;
}

const TEST_CONST = TestEnum::EnumCase;

$value = TEST_CONST;
