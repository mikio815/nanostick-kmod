# nanostick-kmod

2軸ジョイスティックを用いたポインティングデバイス用のカーネルモジュール

- 機材
    - arduino nano


## ロード

```sh
make
sudo insmod fake_mouse.ko
```

マウスを止める

```sh
sudo rmmod fake_mouse
```
