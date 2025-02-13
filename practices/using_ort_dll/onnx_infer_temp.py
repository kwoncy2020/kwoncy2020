from PIL import Image
import onnxruntime as ort
import numpy as np

image = Image.open("11_0_test_resized.jpg")

image_np = np.array(image)
image_np_1 = np.expand_dims(image,0)
ort_session = ort.InferenceSession("my_modified.onnx")
g_inputs = ort_session.get_inputs()
# outputs = ort_session.run(None,{g_inputs[0].name:image_np_1})
# outputs = ort_session.run(["154"],{g_inputs[0].name:image_np_1})
# print(outputs)
# out_np = outputs[0][0]
# out_np_1 = (out_np * 255)
# out_np_2 = np.transpose(out_np_1.astype(np.uint8),[1,2,0])
# Image.fromarray(out_np_2).save("my_modified_output.jpg")

outputs = ort_session.run(None,{g_inputs[0].name:image_np_1})
out_np = outputs[0][0]
Image.fromarray(out_np).save("my_modified_output.jpg")
