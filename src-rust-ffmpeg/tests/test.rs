use clipengine::core::av_decoder::{ AVDecoder };

#[test]
fn test_get_version() {
    let version = AVDecoder::get_version();
    println!("########## {:?}", version);
    assert!(version == "N-120411-gf09c834a7d-20250730");
}

#[test]
fn test_get_av_meta_data() {
    let av_metadata = AVDecoder::get_av_meta_data("E://test.MP4");
    println!("########## {:?}", av_metadata);
    assert!(av_metadata.duration > 0.0);
}



