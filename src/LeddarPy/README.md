# leddar

Python wrapper for LeddarTech SDK.

## Usage

```python
import leddar
d = leddar.Device()
d.connect('192.168.0.20')
print(d.get_echoes())
```