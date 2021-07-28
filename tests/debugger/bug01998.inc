<?php
enum Currency: string {
    case EUR = "€";
    case GBP = "£";
}

final class TotalAmount
{
    private int $amount;

    private Currency $currency;

    public function __construct(int $amount, Currency $currency)
    {
        $this->amount = $amount;
        $this->currency = $currency;
    }
}

$totalAmount = new TotalAmount(30, Currency::EUR);

echo "Hi!\n";
?>
