CREATE TABLE car_prices (
    year UInt16,
    make String,
    model String,
    trim String,
    body String,
    transmission String,
    vin String,
    state String,
    condition UInt8,
    odometer UInt32,
    color String,
    interior String,
    seller String,
    mmr UInt32,
    sellingprice UInt32,
    saledate DateTime
) ENGINE = MergeTree()
ORDER BY (vin);

