import qrcode

url = "https://github.com/rfreyes4/hack4change-TeamKbros"

qr = qrcode.QRCode(
    version=None,
    error_correction=qrcode.constants.ERROR_CORRECT_H,
    box_size=12,
    border=4
)

qr.add_data(url)
qr.make(fit=True)

img = qr.make_image(fill_color="black", back_color="white").convert("RGB")
img.save("qr_github_gestus.png")

print("QR generado correctamente: qr_github_gestus.png")