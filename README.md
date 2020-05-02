# Obtaining

The GPL source code for the RV34x was obtained by emailing [external-opensource-requests@cisco.com](mailto:external-opensource-requests@cisco.com) using the reference number `78EE117C99-195858235` as stated in the documentation ([PDF](https://www.cisco.com/c/dam/en/us/td/docs/routers/csbr/RV340/OSD/RV34xRouters103xv10-2.pdf)).

# Building

## OpenWrt components

```
cd src-comcerto-openwrt-ls1024_7.0.0-rc0-gpl
cp configs/config-ciscosbr-c2krv34x .config
make V=99
```
