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
sudo tools/ns_pty_test --frame --ly 40 --ry 40   # Y同方向でスクロール
sudo tools/ns_pty_test --frame --lx 50 --rx -50  # X逆方向でズーム
sudo tools/ns_pty_test --frame --lx 20 --ly 5    # 通常移動
sudo tools/ns_pty_test --frame --buttons 1       # 左クリック押下
sudo tools/ns_pty_test --frame --buttons 0       # 左クリック解放
```

実デバイスに適用（例: /dev/ttyUSB0）

```sh
sudo tools/ns_ldisc_ctl /dev/ttyUSB0 set
```

## systemd/udev での自動 ldisc 適用例

`systemd/` ディレクトリにサンプルを置いています（CH340用）

1. ヘルパーを配置  
   `make -C tools && sudo install -m 0755 tools/ns_ldisc_ctl /usr/local/bin/`
2. udev ルールとサービスを設置  
   `sudo install -m 0644 systemd/99-nanostick-ldisc.rules /etc/udev/rules.d/`  
   `sudo install -m 0644 systemd/nanostick-ldisc@.service /etc/systemd/system/`
3. 反映  
   `sudo systemctl daemon-reload`  
   `sudo udevadm control --reload-rules`  
   `sudo udevadm trigger -s tty`

挿すと `nanostick-ldisc@ttyUSB0.service` が自動起動して `TIOCSETD` を実行し、抜くと reset します
