## 컨트롤러
1. Controller를 추가하여 GUI부분에서 관심사를 분리할 예정입니다.
2. 파일로드로 얻게된 이미지데이터에 대한 정보는 ImageData클래스가 관리합니다.
3. Controller는 Repository를 통해 ImageData를 관리합니다.
4. Controller는 GUI와 독립적으로 동작해야 합니다.

## ScrolledImageWindow
1. Controller를 통해 화면에 그릴 이미지 데이터를 얻습니다.
2. 매번 그릴때마다 데이터를 얻어오면 오버헤드가 발생하니, 멤버변수에 저장해두고 변경해야 할 필요가 있을때만 변경하는 식으로 해서 리소스사용량을 줄여야합니다.