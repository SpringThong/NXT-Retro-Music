//! Decoded 44.1 kHz stereo PCM to NXT's existing SDL/DSP/output engine.
//! Separate bounded SOCK_SEQPACKET channel; stdout remains JSON only.
use std::{os::{fd::FromRawFd,unix::net::UnixDatagram},sync::{Arc,atomic::{AtomicU32,Ordering}},time::Duration};
use librespot_playback::{audio_backend::{Sink,SinkError,SinkResult},decoder::AudioPacket,convert::Converter};
pub struct PcmSink { socket:UnixDatagram, generation:Arc<AtomicU32>, epoch:u32 }
impl PcmSink {
 pub fn new(socket:UnixDatagram,generation:Arc<AtomicU32>)->Self{Self{socket,generation,epoch:0}}
 pub fn channel()->Result<UnixDatagram,String>{
  let fd=std::env::var("NXT_SPOTIFY_PCM_FD").ok().and_then(|v|v.parse::<i32>().ok()).filter(|n|*n==3).ok_or("pcm_channel_missing")?;
  // FD 3 is inherited exactly once from the app; never stdin/stdout/stderr.
  let socket=unsafe{UnixDatagram::from_raw_fd(fd)};
  socket.set_write_timeout(Some(Duration::from_secs(3))).map_err(|_|"pcm_channel_invalid")?;
  Ok(socket)
 }
}
impl Sink for PcmSink {
 fn start(&mut self)->SinkResult<()>{self.epoch=self.generation.load(Ordering::Acquire);Ok(())}
 fn write(&mut self,packet:AudioPacket,_:&mut Converter)->SinkResult<()>{
  let AudioPacket::Samples(samples)=packet else{return Err(SinkError::InvalidParams("Expected decoded PCM".into()));};
  for chunk in samples.chunks(2048){
   let mut bytes=Vec::with_capacity(8+chunk.len()*4);bytes.extend_from_slice(&self.epoch.to_le_bytes());bytes.extend_from_slice(&(chunk.len() as u32).to_le_bytes());
   for &s in chunk{bytes.extend_from_slice(&(s as f32).to_le_bytes());}
   let count=self.socket.send(&bytes).map_err(|e|SinkError::OnWrite(e.to_string()))?;
   if count!=bytes.len(){return Err(SinkError::OnWrite("Incomplete PCM packet".into()));}
  }
  Ok(())
 }
}
#[cfg(test)]mod tests{
 use super::*;
 #[test]fn framed_pcm_and_generation(){
  let (tx,rx)=UnixDatagram::pair().unwrap();let stream_generation=Arc::new(AtomicU32::new(7));let mut s=PcmSink::new(tx,stream_generation.clone());let mut converter=Converter::new(None);
  s.start().unwrap();s.write(AudioPacket::Samples(vec![0.25,-0.5]),&mut converter).unwrap();
  let mut b=[0u8;64];let n=rx.recv(&mut b).unwrap();assert_eq!(n,16);assert_eq!(u32::from_le_bytes(b[..4].try_into().unwrap()),7);assert_eq!(f32::from_le_bytes(b[8..12].try_into().unwrap()),0.25);
  stream_generation.store(8,Ordering::Release);s.start().unwrap();s.write(AudioPacket::Samples(vec![1.,-1.]),&mut converter).unwrap();rx.recv(&mut b).unwrap();assert_eq!(u32::from_le_bytes(b[..4].try_into().unwrap()),8);
 }
}
