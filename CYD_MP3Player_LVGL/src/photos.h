#ifndef _PICTURES_H_
#define _PICTURES_H_

LV_IMAGE_DECLARE(img_album);
LV_IMAGE_DECLARE(Annie_Spratt);
LV_IMAGE_DECLARE(BoliviaInteligente);
LV_IMAGE_DECLARE(David_Becke);
LV_IMAGE_DECLARE(Doncoombez);
LV_IMAGE_DECLARE(Dzo);
LV_IMAGE_DECLARE(Esra_Afsar);
LV_IMAGE_DECLARE(Grigorii_Shcheglov);
LV_IMAGE_DECLARE(Pranav_Nav);
LV_IMAGE_DECLARE(Rohit_Choudhari);
LV_IMAGE_DECLARE(Vincent_Tint);

const lv_image_dsc_t *pictures[] = {
  /*  0 */  & img_album,
  /*  1 */  & Annie_Spratt,
  /*  2 */  & BoliviaInteligente,
  /*  3 */  & David_Becke,
  /*  4 */  & Doncoombez,
  /*  5 */  & Dzo,
  /*  6 */  & Esra_Afsar,
  /*  7 */  & Grigorii_Shcheglov,
  /*  8 */  & Pranav_Nav,
  /*  9 */  & Rohit_Choudhari,
  /* 10 */  & Vincent_Tint,
};

#define N_PICTURES  (sizeof(pictures) / sizeof(lv_image_dsc_t *))

#endif // _PICTURES_H_