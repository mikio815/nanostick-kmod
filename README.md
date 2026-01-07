# nanostick-kmod

2軸ジョイスティックを用いたポインティングデバイス用のカーネルモジュール

- 機材
    - arduino nano + CH340


## ビルド

```sh
make
```

## ロード

```sh
sudo insmod nanostick.ko
```

## アンロード

```sh
sudo rmmod nanostick
```

## テスト

```sh
make -C tools
```

PTYで擬似入力

```sh
sudo tools/ns_pty_test --frame
```

実デバイスに適用（例: /dev/ttyUSB0）

```sh
sudo tools/ns_ldisc_ctl /dev/ttyUSB0 set
```
